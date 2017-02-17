/**
 * *****************************************************************************
 * License 
 */

/*
 * 
 */
package EoT_MQTT;


import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.FileOutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.sql.Timestamp;
import java.util.Arrays;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.imageio.ImageIO;
import javax.swing.ImageIcon;

import paho.client.mqttv3.*;
import paho.client.mqttv3.persist.*;



/**
 * The EoT MQTT Client.
 */
public class EoT_MQTT_Client implements MqttCallback {

    
    // Broker params
    private String brokerUrl;
    private boolean quietMode = true;
    private String message = "EOT - MQTT Client";
    private String clientId = "EOT_Desktop_Client";
    private boolean cleanSession = true;	// Non durable subscriptions
    private String protocol = "tcp://";
    private String url;
    private int qos = 0;
    private int port;
    
    // Reserved topics
    public final String topicSnapshot = "EOTSnapshot";
    public final String topicEOTConnectToAP = "EOTConnectToAP";
    public final String topicEOTDisconnectFromAP = "EOTDisconnectFromAP";
    public final String topicEOTCreateAP = "EOTCreateAP";
    public final String topicEOTDownloadFileSD = "EOTDownloadFileSD";
    public final String topicEOTUploadFileSD = "EOTUploadFileSD";
    public final String topicEOTUploadElf = "EOTUploadElf";
    public final String topicEOTUpdateDate = "EOTUpdateDate";
    public final String topicEOTGetDate = "EOTGetDate";
    public final String topicEOTListFilesSD = "EOTListFilesSD";
    public final String topicEOTDeleteFileSD = "EOTDeleteFileSD";
    public final String topicEOTDeleteDirSD = "EOTDeleteDirSD";
    public final String topicEOTContentSD = "EOTDeleteContentSD";
    public final String topicEOTMakeDirSD = "EOTMakeDirSD";

    // Private instance variables
    private MqttClient client;
    private MqttConnectOptions conOpt;
    private EoT_MainFrame mainFrame;
    private String currentDeviceDate;
    private String paths[];
    private String destPath;
    private int result; 
    /*Variables used to order received packages*/
    private byte[] fileData = null;
    private int fileLength = 0;
    private int actual_packet = 0;
    private boolean first_package = true;
    /* Image*/
    ImageIcon image; 
    
    // This variable is set to true when the action is completed
    private volatile boolean operationDone;
    private volatile int send_next;
    
    
    
    /**
     * Gets an instance of EoT_MQTT_Client
     * @param brokerip IP where the broker is running
     * @param brokerport port used by the broker
     */
     public EoT_MQTT_Client(String brokerip, int brokerport) {
        // Messages are temporarily stored until
        // they have been delivered to the server.
        String tmpDir = System.getProperty("java.io.tmpdir");
        MqttDefaultFilePersistence dataStore = new MqttDefaultFilePersistence(tmpDir);
        
        this.brokerUrl = brokerip;
        this.port = brokerport;
        this.url = this.protocol + this.brokerUrl + ":" + this.port;

        try {
    	    // Construct the connection options object
            conOpt = new MqttConnectOptions();
            conOpt.setCleanSession(this.cleanSession);
            conOpt.setKeepAliveInterval(0);
            // Construct an MQTT blocking mode client
            client = new MqttClient(this.url, this.clientId+this.toString(), dataStore);
            //client = new MqttClient(this.url, this.clientId+this.toString(), null);
	    // Set this wrapper as the callback handler
            client.setCallback(this);
        } catch (MqttException e) {
            e.printStackTrace();
            log("Unable to set up the client: " + e.toString());
        }
    }

    /**
     * Connects the client to the MQTT server
     *
     * @throws MqttException
     */
    public void connect() throws MqttException{
        log("Connecting to " + this.brokerUrl + " with client ID " + client.getClientId());
        client.connect(conOpt);
        log("Connected");
    }

    /**
     * Disconnects the client
     *
     * @throws MqttException
     */
    public void disconnect() throws MqttException{
        if(client!=null && client.isConnected()){
            client.disconnect();
            log("Disconnected");
        }
    }

    /**
     * Publishes / sends a message to an MQTT server
     *
     * @param topicName the name of the topic to publish to
     * @param qos the quality of service to delivery the message at (0,1,2) (0 in this case)
     * @param payload the set of bytes to send to the MQTT server
     * @throws MqttException
     */
    public void publish(String topicName, int qos, byte[] payload) throws MqttException {
        
        String time = new Timestamp(System.currentTimeMillis()).toString();
        log("Publishing at: " + time + " to topic \"" + topicName + "\" qos " + qos);

        // Create and configure a message
        MqttMessage message = new MqttMessage(payload);
        message.setQos(qos);

    	// Send the message to the server, control is not returned until
        // it has been delivered to the server meeting the specified
        // quality of service.
        client.publish(topicName, message);

    }

    /**
     * Subscribes the client to a topic on an MQTT server 
     *
     * @param topicName to subscribe to (can be wild carded)
     * @param qos the maximum quality of service to receive messages at for this
     * subscription
     * @throws MqttException
     */
    public void subscribe(String topicName, int qos) throws MqttException {

    	// Subscribe to the requested topic
        // The QoS specified is the maximum level that messages will be sent to the client at.
        // For instance if QoS 1 is specified, any messages originally published at QoS 2 will
        // be downgraded to 1 when delivering to the client but messages published at 1 and 0
        // will be received at the same level they were published at.
        log("Subscribing to topic \"" + topicName + "\" qos " + qos);
        client.subscribe(topicName, qos);
        log("Subscribed");

    }

    /**
     * Utility method to handle logging. If 'quietMode' is set, this method does
     * nothing
     *
     * @param message the message to log
     */
    private void log(String message) {
        if (!this.quietMode) {
            System.out.println(message);
        }
    }

    
    
    
    /**
     * *************************************************************
     *
     * Methods to implement the MqttCallback interface              
     *
     * *************************************************************
     */
    
    
    /**
     * @param cause
     * @see MqttCallback#connectionLost(Throwable)
     */
    @Override
    public void connectionLost(Throwable cause) {
	// Called when the connection to the server has been lost.
        // An application may choose to implement reconnection
        // logic at this point. This sample simply exits.
        log("Connection to " + this.brokerUrl + " lost!" + cause);
    }

    /**
     * @param topic
     * @param messageArrived
     * @throws java.lang.Exception
     * @see MqttCallback#messageArrived(String, MqttMessage)
     */
    @Override
    public void messageArrived(String topic, MqttMessage messageArrived) throws Exception {
        // Called when a message arrives from the server that matches any
        // subscription made by the client
        log("Topic "+topic+": "+new String(messageArrived.getPayload()));
       
        switch (topic) {
            case (topicSnapshot):
                // Receives part of the snapshot
                if (first_package == true) {
                    fileLength = Integer.parseInt(new String(messageArrived.getPayload()));
                    first_package = false;
                    log("Image received, to receive " + fileLength + " packets");
                } else {
                    log("Packet " + actual_packet + " received");
                    if (actual_packet == 0) {
                        fileData = messageArrived.getPayload();
                    } else {
                        byte[] b = messageArrived.getPayload();
                        byte[] c = new byte[fileData.length + b.length];
                        System.arraycopy(fileData, 0, c, 0, fileData.length);
                        System.arraycopy(b, 0, c, fileData.length, b.length);
                        fileData = c;
                    }
                    actual_packet++;
                }
                if (actual_packet == fileLength) {
                    log("Showing picture");
                    BufferedImage bimg = ImageIO.read(new ByteArrayInputStream(fileData));
                    image = new ImageIcon();
                    image.setImage(bimg);
                    actual_packet = 0;
                    first_package = true;
                    operationDone = true;
                }
                break;
            case (topicEOTDownloadFileSD):
                // Receives part of the file
                if (first_package == true) {
                    fileLength = Integer.parseInt(new String(messageArrived.getPayload()));
                    first_package = false;
                    log("File received, to receive " + fileLength + " packets");
                } else {
                    log("Packet " + actual_packet + " received");
                    if (actual_packet == 0) {
                        fileData = messageArrived.getPayload();
                    } else {
                        byte[] b = messageArrived.getPayload();
                        byte[] c = new byte[fileData.length + b.length];
                        System.arraycopy(fileData, 0, c, 0, fileData.length);
                        System.arraycopy(b, 0, c, fileData.length, b.length);
                        fileData = c;
                    }
                    actual_packet++;
                }
                if (actual_packet == fileLength) {
                    log("Saving file...");
                    //Write file with data
                    FileOutputStream fos = new FileOutputStream(this.destPath);
                    byte[] data = fileData;
                    fos.write(data);
                    fos.close();
                    log("File saved!");
                    actual_packet = 0;
                    first_package = true;
                    operationDone = true;
                    this.mainFrame.showDialog("File received!");
                }
                break;
            case (topicEOTUploadFileSD):
                // Receives the result of uploading a file to the SD card
                result = Integer.parseInt(new String(messageArrived.getPayload()));
                if (result == 2){
                    this.send_next=1;
                }
                else {
                    this.operationDone = true;
                }
                break;
            case (topicEOTGetDate):
                // Receives the device date
                currentDeviceDate = new String(messageArrived.getPayload());
                this.operationDone = true;
                break;
            case (topicEOTUpdateDate):
                // Receives the result of updating the date
                result = Integer.parseInt(new String(messageArrived.getPayload()));
                this.operationDone = true;
                break;
            case (topicEOTUploadElf):
                // Receives the result of uploading an el file to the flash memmory
                result = Integer.parseInt(new String(messageArrived.getPayload()));
                if (result == 2){
                    this.send_next=1;
                }
                else {
                    this.operationDone = true;
                }
                break;
            case (topicEOTListFilesSD):
                // Receives part of the SD card files paths
                if (first_package == true) {
                    int packs = Integer.parseInt(new String(messageArrived.getPayload()));
                    fileLength = packs;
                    log(packs + "");
                    this.paths = new String[packs];
                    first_package = false;
                    actual_packet = 0;
                    log("el primero");
                } else {
                    log("Packet " + actual_packet + " received\n");
                    this.paths[this.actual_packet] = new String(messageArrived.getPayload());
                    actual_packet++;
                }
                if (actual_packet == fileLength) {
                    this.operationDone = true;
                    actual_packet = 0;
                    first_package = true;
                }
                break;
            case (topicEOTMakeDirSD):
            case (topicEOTDeleteFileSD):
            case (topicEOTContentSD):
            case (topicEOTDeleteDirSD):
                result = Integer.parseInt(new String(messageArrived.getPayload()));
                this.operationDone = true;
                break;
            default:
                // Messages in other topics (these are shown in the Graphical Interface
                mainFrame.showMessage(topic, messageArrived.toString());
                break;

        }
        
    }

    
    /**
     * @param token
     * @see MqttCallback#deliveryComplete(IMqttDeliveryToken)
     */
    @Override
    public void deliveryComplete(IMqttDeliveryToken token) {
	// Called when a message has been delivered to the
        // server. The token passed in here is the same one
        // that was passed to or returned from the original call to publish.
        // This allows applications to perform asynchronous
        // delivery without blocking until delivery completes.
        //
        // This sample demonstrates asynchronous deliver and
        // uses the token.waitForCompletion() call in the main thread which
        // blocks until the delivery has completed.
        // Additionally the deliveryComplete method will be called if
        // the callback is set on the client
        //
        // If the connection to the server breaks before delivery has completed
        // delivery of a message will complete after the client has re-connected.
        // The getPendingTokens method will provide tokens for any messages
        // that are still to be delivered.
    }

    
    
    
    
    
    
    /**
     * *************************************************************
     *
     * SD card Management Methods        
     *
     * *************************************************************
     */
    

    /**
     * Sets the main frame where results are displayed
     * @param frame
     */
    public void setMainFrame(EoT_MainFrame frame) {
        mainFrame = frame;
    }

    /**
     * Unsubscribes the client from a topic
     * @param topicName
     * @throws MqttException
     */
    public void unsubscribe(String topicName) throws MqttException {

    	log("Unsubscribing to topic \"" + topicName);
        client.unsubscribe(topicName);
        log("Unsubscribed");
        
    }

    /**
     * Sends a message in the topicSnapshot topic to get the image from the broker
     * @throws MqttException
     */
    public ImageIcon askSnapshot() throws MqttException{
        // Ask for the snapshot
        this.operationDone = false;
        subscribe(this.topicSnapshot, this.qos);
        try {
            Thread.sleep(100);
        } catch (InterruptedException ex) {
            Logger.getLogger(EoT_MainFrame.class.getName()).log(Level.SEVERE, null, ex);
        }
        publish(this.topicSnapshot, this.qos, this.message.getBytes());
        //sampleClient.unsubscribe(topicSnapshot);
        while (!this.operationDone);
        return this.image;
    }
    
    /**
     * Sends a file to the SD card
     * @param srcDir Path of the file
     * @param dstName SD card path where the file should be store
     * @return 0 if all is OK
     * @throws Exception
     */
    public int uploadFile(String srcDir, String dstName) throws Exception{
        
        this.operationDone = false;
        this.send_next = 1;
        subscribe(this.topicEOTUploadFileSD, this.qos);
        Path path = Paths.get(srcDir);
        byte[] data = Files.readAllBytes(path);
        MqttMessage message1;// = new MqttMessage(data);
        log("Content length in bytes " + data + " " + data.length);
        //add 1024 char limit to payload
        int total_length = 1024;
        //message 1 (buffer length)
        MqttMessage message2 = null;
        if( (data.length%total_length) == 0){
            message2 = new MqttMessage((Integer.toString(data.length / total_length) + " " + dstName).getBytes());
        }else{
            message2 = new MqttMessage((Integer.toString(data.length / total_length + 1) + " " + dstName).getBytes());
        }
        message2.setQos(qos);
        
        client.publish(topicEOTUploadFileSD, message2);

        //int sleep_time = 100;
        int length_sent = 0;
        int actual_packet_sent = 0;
        while (length_sent < data.length && !this.operationDone) {
            while (this.send_next==0);
            this.send_next=0;
            //Thread.sleep(sleep_time); //sleep 0.1 sec (10KB/s)
            //add number of package (6 characters)
            actual_packet_sent = length_sent / total_length;
            String number_6zeros = String.format("%06d", actual_packet_sent);

            //create message (last 6 characters are the number of the package)
            byte[] subData1 = Arrays.copyOfRange(data, length_sent, Math.min(length_sent + total_length, data.length));
            byte[] subData2 = number_6zeros.getBytes();

            byte[] subData = new byte[subData1.length + subData2.length];
            System.arraycopy(subData1, 0, subData, 0, subData1.length);
            System.arraycopy(subData2, 0, subData, subData1.length, subData2.length);

            message1 = new MqttMessage(subData);
            message1.setQos(qos);
            client.publish(topicEOTUploadFileSD, message1);
            length_sent += total_length;
            System.out.println("Message  " + length_sent / total_length + " published");
        }
        while (!this.operationDone);
        return result;
    }
    
    /**
     * Downloads a file from the SD card
     * @param srcDir SD card path of the file
     * @param dstDir Path where the file should be store
     * @throws Exception
     */
    public void downloadFile(String srcDir, String dstDir) throws Exception {
        this.operationDone = false;
        subscribe(this.topicEOTDownloadFileSD, this.qos);
        try {
            Thread.sleep(100);
        } catch (InterruptedException ex) {
            Logger.getLogger(EoT_MainFrame.class.getName()).log(Level.SEVERE, null, ex);
        }
        destPath=dstDir;
        publish(this.topicEOTDownloadFileSD, this.qos, (srcDir).getBytes());
        while (!this.operationDone);
    }
    
    /**
     * Checks if the client is connected
     * @return true if the client is connected.
     */
    public boolean isConnected(){
        return client.isConnected();
    }

    /**
     * Gets the paths of the SD card content
     * @return A String[] with all the file and folder paths
     */
    public String[] getFileSystemStructure(String path) throws MqttException {
        this.operationDone =  false;
        first_package = true;
        subscribe(this.topicEOTListFilesSD, this.qos);
        publish(this.topicEOTListFilesSD, this.qos, (path).getBytes());
        while(!operationDone);
        return this.paths;
    }

    /**
     * Makes a new folder in the SD card
     * @param path Path of the new folder
     * @return 0 if the operation was successfully completed
     */
    public int createFolder(String path) throws MqttException {
        this.operationDone = false;
        subscribe(this.topicEOTMakeDirSD, this.qos);
        publish(this.topicEOTMakeDirSD, this.qos, (path).getBytes());
        while(!operationDone);
        return this.result;
    }

    /**
     * Removes a file from the SD card
     * @param path Path of the file to be removed
     * @return 0 if the operation was successfully completed
     */
    public int removeFile(String path) throws MqttException {
        this.operationDone = false;
        subscribe(this.topicEOTDeleteFileSD, this.qos);
        publish(this.topicEOTDeleteFileSD, this.qos, (path).getBytes());
        while(!operationDone);
        return this.result;
    }
    

    /**
     * Removes the content of a folder (or the SD card if /mnt/sdcard is used)
     * @param path Path of the folder
     * @return 0 if the operation was successfully completed
     */
    public int removeContent(String path) throws MqttException {
        this.operationDone = false;
        subscribe(this.topicEOTContentSD, this.qos);
        publish(this.topicEOTContentSD, this.qos, (path).getBytes());
        while(!operationDone);
        return this.result;
    }

    /**
     * Removes a folder and its content recursively
     * @param path Path of the folder
     * @return 0 if the operation was successfully completed
     */
    public int removeAll(String path) throws MqttException {
        this.operationDone = false;
        subscribe(this.topicEOTDeleteDirSD, this.qos);
        publish(this.topicEOTDeleteDirSD, this.qos, (path).getBytes());
        while(!operationDone);
        return this.result;
    }

    
    
    
    
    
    
    /**
     * *************************************************************
     *
     * EoT Device WiFi Connection Settings       
     *
     * *************************************************************
     */
    
   
    
    
    
    /**
     * Resets the AP configuration to the default profile
     * @throws MqttException
     */
    public void resetAPConfig() throws MqttException {
        //EOTDisconnectFromAP
        subscribe(this.topicEOTDisconnectFromAP, this.qos);
        try {
            Thread.sleep(100);
        } catch (InterruptedException ex) {
            Logger.getLogger(EoT_MainFrame.class.getName()).log(Level.SEVERE, null, ex);
        }
        publish(this.topicEOTDisconnectFromAP, this.qos, ("").getBytes());
    }

    /**
     * Creates a new AP configuration profile
     * @param SSID
     * @param security
     * @param pass
     * @param channel
     * @throws MqttException
     */
    public void createAP(String SSID, String security, String pass, String channel) throws MqttException {
        //EOTCreateAP
        subscribe(this.topicEOTCreateAP, this.qos);
        try {
            Thread.sleep(100);
        } catch (InterruptedException ex) {
            Logger.getLogger(EoT_MainFrame.class.getName()).log(Level.SEVERE, null, ex);
        }
        publish(this.topicEOTCreateAP, this.qos, (SSID+" "+security+" "+pass+" "+channel).getBytes());
    }
    /**
     * Connects the EoT device to an external AP
     * @param SSID
     * @param security
     * @param pass
     * @throws MqttException
     */
    public void connectToAP(String SSID, String security, String pass) throws MqttException {
        //EOTConnectToAP
        subscribe(this.topicEOTConnectToAP, this.qos);
        try {
            Thread.sleep(100);
        } catch (InterruptedException ex) {
            Logger.getLogger(EoT_MainFrame.class.getName()).log(Level.SEVERE, null, ex);
        }
        publish(this.topicEOTConnectToAP, this.qos, (SSID+" "+security+" "+pass).getBytes());
    }


    
    
    
    
    
    
    /**
     * *************************************************************
     *
     * EoT Time/Date Settings       
     *
     * *************************************************************
     */
    
   
    
    
    /**
     * Changes the EoT device time/date
     * @param year
     * @param month
     * @param day
     * @param hour
     * @param mins
     * @param secs
     * @throws MqttException
     */
    public int updateDate(String year, String month, String day, String hour, String mins, String secs) throws MqttException {
        //EOTUpdateDate
        this.operationDone = false;
        subscribe(this.topicEOTUpdateDate, this.qos);
        try {
            Thread.sleep(100);
        } catch (InterruptedException ex) {
            Logger.getLogger(EoT_MainFrame.class.getName()).log(Level.SEVERE, null, ex);
        }
        String date = year + " " 
                + month+ " " 
                + day + " "
                + hour + " "
                + mins + " "
                + secs ;
        publish(this.topicEOTUpdateDate, this.qos, date.getBytes());
        while(!operationDone);
        return this.result;
    }

    
    /**
     * Gets the current EoT device time/date
     * @return Calenda The device current time/date 
     * @throws MqttException
     */
    public Calendar getDate() throws MqttException {
        this.operationDone = false;
        subscribe(this.topicEOTGetDate, this.qos);
        try {
            Thread.sleep(100);
        } catch (InterruptedException ex) {
            Logger.getLogger(EoT_MainFrame.class.getName()).log(Level.SEVERE, null, ex);
        }
        publish(this.topicEOTGetDate, this.qos, ("").getBytes());
        while (!this.operationDone);
        log(currentDeviceDate);
        Calendar curDevDate = new GregorianCalendar();
        String[] data = this.currentDeviceDate.split("\\s+");
        
        log("prev "+data[4]+" "+data[2]+" "+data[3].substring(0,2)+" "+data[3].substring(3,5)+" "+data[3].substring(6));
        
        
        int year = Integer.parseInt(data[4]);
        int day = Integer.parseInt(data[2]);
        int hourOfDay = Integer.parseInt(data[3].substring(0, 2));
        int minute = Integer.parseInt(data[3].substring(3, 5));
        int second = Integer.parseInt(data[3].substring(6));
        int month = 0;
        switch(data[1]){
            case "Jan":
                month = 0;
                break;
            case "Feb":
                month = 1;
                break;
            case "Mar":
                month = 2;
                break;
            case "Apr":
                month = 3;
                break;
            case "May":
                month = 4;
                break;
            case "Jun":
                month = 5;
                break;
            case "Jul":
                month = 6;
                break;
            case "Aug":
                month = 7;
                break;
            case "Sep":
                month = 8;
                break;
            case "Oct":
                month = 9;
                break;
            case "Nov":
                month = 10;
                break;
            case "Dec":
                month = 11;
                break;
        }
        log("hola"+year+" "+day+" "+month+" "+hourOfDay+" "+minute+" "+second);
        curDevDate.set(year, month, day, hourOfDay, minute, second);
        
        return curDevDate;
    }

    
    
    
    
    
    
    
    
    /**
     * *************************************************************
     *
     * EoT Flash   
     *
     * *************************************************************
     */
    
   
    /**
     * Sends a program to the flash memmory
     * @param srcDir Path of the program file
     * @return 0 if all is OK
     * @throws Exception
     */
    int uploadProgram(String srcDir) throws Exception {

        this.operationDone = false;
        this.send_next = 1;
        subscribe(this.topicEOTUploadElf, this.qos);
        //TODO: subscribe and unsubscribe in order to get feedback
        Path path = Paths.get(srcDir);
        byte[] data = Files.readAllBytes(path);
        MqttMessage message1;// = new MqttMessage(data);
        log("Content length in bytes " + data + " " + data.length);
        //add 1024 char limit to payload
        int total_length = 1024;
        //message 1 (buffer length)
        MqttMessage message2 = null;
        if( (data.length%total_length) == 0){
            message2 = new MqttMessage((Integer.toString(data.length / total_length) + " dummy.d").getBytes());
        }else{
            message2 = new MqttMessage((Integer.toString(data.length / total_length + 1) + " dummy.d").getBytes());
        }
        message2.setQos(qos);
        
        client.publish(topicEOTUploadElf, message2);

        //int sleep_time = 100;
        int length_sent = 0;
        int actual_packet_sent = 0;
        while (length_sent < data.length && !this.operationDone) {
            
            while (this.send_next==0);
            this.send_next=0;
            //Thread.sleep(sleep_time); //sleep 0.1 sec (10KB/s)
            //add number of package (6 characters)
            actual_packet_sent = length_sent / total_length;
            String number_6zeros = String.format("%06d", actual_packet_sent);

            //create message (last 6 characters are the number of the package)
            byte[] subData1 = Arrays.copyOfRange(data, length_sent, Math.min(length_sent + total_length, data.length));
            byte[] subData2 = number_6zeros.getBytes();

            byte[] subData = new byte[subData1.length + subData2.length];
            System.arraycopy(subData1, 0, subData, 0, subData1.length);
            System.arraycopy(subData2, 0, subData, subData1.length, subData2.length);

            message1 = new MqttMessage(subData);
            message1.setQos(qos);
            client.publish(topicEOTUploadElf, message1);
            length_sent += total_length;
            log("Message  " + length_sent / total_length + " published");
        }
        while (!this.operationDone);
        return result;
    }

    
    
}



