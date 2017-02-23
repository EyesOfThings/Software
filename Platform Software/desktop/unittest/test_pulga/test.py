#!/usr/bin/python -u
# -*- coding: utf-8 -*-

import unittest
import wifiUtils
import paho.mqtt.client as mqtt
import logging
import os
import filecmp
import datetime
import time

def wait():
    time.sleep(0.5)


class PulgaTests(unittest.TestCase):
    SSID = "Myriad2Wifi"
    password = "visilabap"
    topic = "visilab"
    serverAddr = "192.168.1.1"
    port = 1883
    keepAlive = 60

    pathTestFile = "test.py"
    downloadedFile = "downloaded"
    pathOnSD = "/mnt/sdcard/" + pathTestFile.split("/")[-1]

    def on_message(self, client, userdata, msg):
        self.message = msg
        # print "message %s" % self.message.payload

    def on_message2(self, client, userdata, msg):
        self.message = msg

    def on_log(self, client, userdata, level, buf):
        logging.info("Log level: %s\n\tMessage: %s\n\tuserdata: %s\n", level, buf, userdata)

    def setUp(self):
        self.message = mqtt.MQTTMessage()
        self.client = mqtt.Client()
        self.client.on_message = self.on_message
        self.client.on_log = self.on_log
        
    def externalSetUp(self):
        self.utils = wifiUtils.wifiUtils("wlan0")
        self.utils.getAPpath(self.SSID)
        self.utils.connectToAP(self.SSID, self.password)  # sometimes it fails here

    def tearDown(self):
        self.client.disconnect()

    def test00ConnectToBroker(self):
        self.externalSetUp()

        self.client.connect(self.serverAddr, self.port, self.keepAlive)  # blocking calls
        self.client.disconnect()
        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        # connect to an already connected server
        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        self.client.disconnect()
        # disconnect to an already disconnected server
        self.client.disconnect()


    def test01SubscribeUnsubscribe(self):
        wait()
        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        #    self.client.connect_async(self.serverAddr, self.port, self.keepAlive)

        self.client.loop_start()  # client start listening for new messages
        # Messages to a subscribed topic
        #print "Subscribed to %s" % (self.topic)

        self.client.subscribe(self.topic)

        wait()
        #print "Client unsubscribe from topic %s" % (self.topic)
        result, mid = self.client.unsubscribe(self.topic)

        print "Check if unsubscription was successful"
        self.assertEqual(result, mqtt.MQTT_ERR_SUCCESS, "error unsubscribing")

    def test02UploadFile(self):
        wait()
        chunkSize = 1024
        fileSize = os.path.getsize(self.pathTestFile)

        numberOfPackages = fileSize / chunkSize
        if fileSize % chunkSize != 0:
            numberOfPackages += 1

        # f = codecs.open(pathTestFile, "r", "utf-8")
        f = open(self.pathTestFile, 'rb')
        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        self.client.loop_start()

        #print "Uploading file %s" % self.pathTestFile
        self.client.publish("EOTUploadFileSD", " %s %s" % (numberOfPackages, self.pathOnSD))
        for pkgIndex in range(numberOfPackages):
            pkgNumString = u"%06d" % (pkgIndex)
            print "Uploading %s chunk" % pkgNumString
            data = "%s%s" % (f.read(chunkSize), pkgNumString)
            self.client.publish("EOTUploadFileSD", data)
        f.close()
        self.assertTrue(True, "pass")

    def test03DownloadFile(self):
        wait()
        #print "Downloading file %s" % self.pathOnSD
        lastMessage = self.message.payload

        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        self.client.loop_start()

        self.client.subscribe("EOTDownloadFileSD")
        f = open(self.downloadedFile, "wb")
        self.client.publish("EOTDownloadFileSD", self.pathOnSD)

        timeout = time.time() + 5
        numberOfPackages = ""
        while 1:
            numberOfPackages = self.message.payload
            if lastMessage != numberOfPackages:
                break
            if time.time() > timeout:
                self.assertTrue(False, "Timeout reached, some packet lost or Pulga disconnected")
        lastMessage = numberOfPackages
        #print "number of packages %s" % numberOfPackages
        try:
            numberOfPackages = int(numberOfPackages)
        except:
            self.assertTrue(False, "Error on received message with size of snapshot")

        timeout = time.time() + 5
        while numberOfPackages > 0:
            # print "Downloading chunk"
            data = self.message.payload
            if lastMessage != data:
                lastMessage = data
                numberOfPackages -= 1
                f.write(self.message.payload)
            if time.time() > timeout:
                self.assertTrue(False, "Timeout reached, some packet lost or Pulga disconnected")

        f.close()

    def test04UploadDownloadFileAreEqual(self):
        wait()
        are_equal = filecmp.cmp(self.pathTestFile, self.downloadedFile)

        # print "Files equals: %s" % are_equal
        os.remove(self.downloadedFile)
        self.assertTrue(are_equal, "Uploaded and Downloaded files are not same file")

    def test05RequestSnapshot(self):
        wait()
        downloadedSnapshot = "downloaded.jpg"
        #snapshotTopic = "Snapshot"
        snapshotTopic = "EOTSnapshot"
        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        self.client.loop_start()

        lastMessage = self.message.payload

        self.client.subscribe(snapshotTopic)
        self.client.publish(snapshotTopic, "")
        f = open(downloadedSnapshot, "wb")

        timeout = time.time() + 5
        numberOfPackages = ""
        while 1:
            numberOfPackages = self.message.payload
            #if numberOfPackages != None:
            #    print numberOfPackages
            if lastMessage != numberOfPackages:
                break
            if time.time() > timeout:
                self.assertTrue(False, "Timeout reached, some packet lost or Pulga disconnected")

        lastMessage = numberOfPackages
        #print "number of packages %s" % numberOfPackages
        try:
            numberOfPackages = int(numberOfPackages)
        except:
            self.assertTrue(False, "Error on received message with size of snapshot")

        timeout = time.time() + 5
        while numberOfPackages > 0:
            #print "Downloading chunk"
            data = self.message.payload
            if lastMessage != data:
                lastMessage = data
                numberOfPackages -= 1
                f.write(self.message.payload)
            if time.time() > timeout:
                self.assertTrue(False, "Timeout reached, some packet lost or Pulga disconnected")

        f.close()

        #os.remove(downloadedSnapshot)


    def test06UpdateDate(self):
        wait()
        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        self.client.loop_start()

        now = datetime.datetime.now()
        year = now.year
        month = now.month
        day = now.day
        hour = now.hour
        minute = now.minute
        second = now.second

        self.client.publish("EOTUpdateDate", "%s %s %s %s %s %s" % (year, month, day, hour, minute, second))

    
    def test07GetDate(self):
        wait()
        months = {"Jan": 1, "Feb": 2, "Mar": 3, "Apr": 4, "May": 5, "Jun": 6, 
                "Jul": 7, "Aug": 8, "Sep": 9, "Oct": 10, "Nov": 11, "Dec": 12}
        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        self.client.loop_start()

        lastMessage = self.message.payload
        now = datetime.datetime.now()
        #dateLocal = (now.year, now.month, now.day, now.hour, now.minute, now.second)
        dateLocal = (now.year, now.month, now.day)

        self.client.subscribe("EOTGetDate")
        self.client.publish("EOTGetDate", "")
        timeout = time.time() + 5
        while 1:
            dateMyriad = self.message.payload
            #if numberOfPackages != None:
            #    print numberOfPackages
            if lastMessage != dateMyriad:
                break
            if time.time() > timeout:
                self.assertTrue(False, "Timeout reached, some packet lost or Pulga disconnected")

        dateMyriad = dateMyriad.split(" ")
        dateMyriad = (int(dateMyriad[-1]), int(months[dateMyriad[1]]), int(dateMyriad[2]))

        for timeLocal, timeMyriad in zip(dateLocal, dateMyriad):
            self.assertEqual(timeLocal, timeMyriad)
    
    def test08EOTMakeDirSD(self):
        wait()

        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        self.client.loop_start()

        self.client.publish("EOTMakeDirSD", "/mnt/sdcard/Test")
    
    def test09EOTListFilesSD(self):
        wait()

        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        self.client.loop_start()

        lastMessage = self.message.payload
        self.client.subscribe("EOTListFilesSD")

        self.client.publish("EOTListFilesSD", "/mnt/sdcard/")

        timeout = time.time() + 5
        while 1:
            numberOfPackages = self.message.payload
            if lastMessage != numberOfPackages:
                lastMessage = numberOfPackages
                break
            if time.time() > timeout:
                self.assertTrue(False, "Timeout reached, some packet lost or Pulga disconnected")

        try:
            numberOfPackages = int(numberOfPackages)
        except:
            self.assertTrue(False, "Error on received message")

        timeout = time.time() + 5
        while numberOfPackages > 0:
            #print "Downloading chunk"
            data = self.message.payload
            if lastMessage != data:
                lastMessage = data
                numberOfPackages -= 1
                #print data
            if time.time() > timeout:
                self.assertTrue(False, "Timeout reached, some packet lost or Pulga disconnected")
    
    def test10EOTDeleteFileSD(self):
        wait()

        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        self.client.loop_start()

        self.client.publish("EOTDeleteFileSD", self.pathOnSD)
    
    
    def test11EOTDeleteDirSD(self):
        wait()

        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        self.client.loop_start()

        self.client.publish("EOTDeleteDirSD", "/mnt/sdcard/Test")
    
    '''
    def test11EOTDeleteContentSD(self):
        # Delete dir content
        wait()

        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        self.client.loop_start()

        self.client.publish("EOTDeleteContentSD", "/mnt/sdcard/")
    '''
    def test12UploadElf(self):
        wait()
        chunkSize = 1024
        fileSize = os.path.getsize(self.pathTestFile)

        numberOfPackages = fileSize / chunkSize
        if fileSize % chunkSize != 0:
            numberOfPackages += 1

        # f = codecs.open(pathTestFile, "r", "utf-8")
        f = open(self.pathTestFile, 'rb')
        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        self.client.loop_start()

        #print "Uploading file %s" % self.pathTestFile
        self.client.publish("EOTUploadElf", " %s %s" % (numberOfPackages, "ElfName"))
        for pkgIndex in range(numberOfPackages):
            pkgNumString = u"%06d" % (pkgIndex)
            print "Uploading %s chunk" % pkgNumString
            data = "%s%s" % (f.read(chunkSize), pkgNumString)
            self.client.publish("EOTUploadFileSD", data)
        f.close()
        self.assertTrue(True, "pass")
    '''
    def test13EOTListElf(self):
        wait()

        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        self.client.loop_start()

        lastMessage = self.message.payload
        self.client.subscribe("EOTListElf")

        self.client.publish("EOTListElf", "-")
        while 1:
            numberOfPackages = self.message.payload
            #if numberOfPackages != None:
            #    print numberOfPackages
            if lastMessage != numberOfPackages:
                break
        try:
            numberOfPackages = int(numberOfPackages)
        except:
            self.assertTrue(False, "Error on received message with size of snapshot")

        while numberOfPackages > 0:
            #print "Downloading chunk"
            data = self.message.payload
            if lastMessage != data:
                lastMessage = data
                numberOfPackages -= 1
                print data

    def test20ConnectToAP(self):
        wait()
        newSSID = "visilab"
        security = 2
        newPassword = "123456789"
        myriadAddr = "192.168.1.6"

        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        self.client.loop_start()
        self.client.publish("EOTConnectToAP"," %s %s %s" % (newSSID, security, newPassword))
        wait()
        wait()
        wait()
        wait()

        self.utils.connectToAP(newSSID,newPassword)
        #self.assertTrue(True,"AP not connected")


        #Test if pulga has been correctly restarted
        self.client.connect(myriadAddr, self.port, self.keepAlive)

    def test21DisconnectFromAP(self):
        wait()
        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        self.client.loop_start()

        self.client.publish("EOTDisconnectFromAP","")

    def test22CreateAP(self):
        wait()
        newSSID = "visilab2"
        newPassword = "987654321"
        security = 2
        channel = 4
        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        self.client.publish("EOTCreateAP"," %s %s %s %s" % (newSSID, security, newPassword, channel))
        wait()
        wait()
        wait()
        wait()

        self.assertNotEqual(self.utils.getAPpath(newSSID),"AP not found","AP not found")
        self.utils.connectToAP(newSSID,newPassword) #connecting to AP is blocking
        #self.assertTrue(True,"AP not connected")


        #Test if pulga has been correctly restarted
        self.client.connect(self.serverAddr, self.port, self.keepAlive)

        # Back to previous state

        channel = 8

        self.client.publish("EOTCreateAP"," %s %s %s %s" % (self.SSID, security, self.password, channel))
        wait()
        wait()
        wait()
        wait()
        wait()

        self.assertNotEqual(self.utils.getAPpath(self.SSID),"AP not found","AP not found")
        self.utils.connectToAP(self.SSID,self.password) #connecting to AP is blocking
        #self.assertTrue(True,"AP not connected")


        #Test if pulga has been correctly restarted
        self.client.connect(self.serverAddr, self.port, self.keepAlive)

    def test23EOTStartElf(self):
        # Delete dir content
        wait()

        self.client.connect(self.serverAddr, self.port, self.keepAlive)
        self.client.loop_start()

        self.client.publish("EOTStartElf", "ElfName")
    '''
def main():
    logging.basicConfig(filename='example.log', level=logging.INFO)
    unittest.main(verbosity=2)


if __name__ == '__main__':
    main()
