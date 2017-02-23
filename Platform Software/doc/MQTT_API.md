#Myriad 2 MQTT commands

The client must subscribe to each reserved topic before sending the first message and unsubscribe after finishing the command.

The package described in all the document corresponds to a packet of information with a maximum size of 1024 bytes. This fact is because of the limitation imposed by the CC3100 WiFi device when sending a message through sockets.

| Basic Description                               | Topic                          | Publish Message                |
|-------------------------------------------------|--------------------------------|--------------------------------|
|  [1. Upload file to SD](#1)                     | EOTUploadFileSD                | NumberOfPackages FileName      |
|  [2. List files from SD](#2)                    | EOTListFilesSD                 | -                              |
|  [3. Download file from SD](#3)                 | EOTDownloadFileSD              | FileName                       |
|  [4. Upload and flash an application (ELF)](#4) | EOTUploadElf                   | NumberOfPackages AppName       |
|  [5. List ELF files](#5)                        | EOTListElf                     | -                              |
|  [6. Run ELF executable](#6)                    | EOTRunElf                      | ElfName                        |
|  [7. Request a camera snapshot](#7)             | EOTSnapshot                    | -                              |
|  [8. Create Access Point](#8)                   | EOTCreateAP                    | Ssid Security [Pass] [Channel] |
|  [9. Connect to Acces Point](#9)                | EOTConnectToAP                 | Ssid Security [Pass]           |
| [10. Disconnect from Acces point](#10)          | EOTDisconnectFromAP            | -                              |
| [11. List WiFi networks](#11)                   | EOTListWifi                    | -                              |
| [12. Update date](#12)                          | EOTUpdateDate                  | Year Month Day Hour Min Sec    |

<a name="1"/>
###1. Upload file to SD
#####Parameters:
* NumberOfPackages: Number of packages to be sent.

* FileName: Name of the file to be written.

#####Functionality:
The client application must send the first message including the defined information. After that the client must send the file divided in parts of maximum 1024 bytes using publish messages with the same topic (EOTUploadFile). The message for each part of the file must be:
* PartOfFIleXXXXXX

Where XXXXXX is the number of the package sent using 6 digits (it must include zeros at left). The complete size of the message would be 1024+6 at maximum.

#####Myriad 2 API:
For receiving the file, the [receive_file](#receiveFile) function is implemented.

#####Answer to the client:
The Myriad 2 must send a message to the client in the same topic indicating if the file has been uploaded correctly or an error.

<a name="2"/>
###2. List files from SD
#####Parameters:
No parameters

#####Functionality:
Send to the client a list with the name of the files uploaded to the device SD.

#####Answer to the client:
The Myriad 2 must send a message to the client in the same topic indicating the files of the device SD.

<a name="3"/>
###3. Download file from SD
#####Parameters:
* FileName: Name of the file to be downloaded.

#####Functionality:
Send to the client the indicated file.

#####Myriad 2 API:
For sending the file, the [send_file](#sendFile) function is implemented.

#####Answer to the client:
First the Myriad 2 must send a message in the same topic as follows:

* NumberOfPackages

This message indicate the number of packages to be received by the client. The size of packages is at maximum 1024 bytes.

After that the Myriad 2 must send NumberOfPackages messages partitioning the file using the same topic. The client should combine this packages to obtain the complete file.

<a name="4"/>
###4.Upload and flash an application (ELF)
#####Parameters:
* NumberOfPackages: Number of packages to be sent.
* AppName: Name of the app to be written.

#####Functionality:
The client application must send the first message including the defined information. After that the client must send the ELF file divided in packages using publish messages with the same topic (EOTUploadElf). The message for each part of the file must be:

* PartOfFIleXXXXXX

Where XXXXXX is the number of the package sent using 6 digits (it must include zeros at left). The complete size of the message would be 1024+6 at maximum.

#####Myriad 2 API:
For receiving the file, the [receive_file](#receiveFile) function is implemented. See next section (API functions Myriad 2) for more information.

#####Answer to the client:
The Myriad 2 must send a message to the client in the same topic indicating if the ELF has been uploaded correctly or an error.

<a name="5"/>
###5. List ELF files
#####Parameters:
No parameters.

#####Functionality:
Send to the client a list with the name of the ELF files uploaded to the device.

#####Myriad 2 API:

#####Answer to the client:
The Myriad 2 must send a message to the client in the same topic indicating the ELF files of the device.

<a name="6"/>
###6. Run ELF executable
#####Parameters:
* ElfName: Name of the elf file to be run.

#####Functionality:
The device run the ElfName file.

#####Myriad 2 API:

#####Answer to the client:
The Myriad 2 does not send any message to the client.

<a name="7"/>
###7. Request a camera snapshot
#####Parameters:
No parameters.

#####Functionality:
Send to the client a snapshot taken by the camera of the device.

#####Myriad 2 API:
For sending the picture file, the [send_file](#sendFile) function can be used.

#####Answer to the client:
First the Myriad 2 must send a message in the same topic as follows:

* NumberOfPackages

This message indicate the number of packages to be received by the client.

After that the Myriad 2 must send NumberOfPackages messages partitioning the picture using the same topic. The client should combine this packages to obtain a complete picture.

<a name="8"/>
###8. Create Access Point
#####Parameters:
* Ssid: SSID of the access point to be created.
Security: Security of the access point. The values of this parameter must be Open, WEP or WPA.
* Pass: Password of the WiFi network if necessary (only on WEP and WPA networks).
* Channel: Channel in which the access point will be emitting. If not defined, it will be selected between the channels less congested with other access points.

#####Functionality:
Create the indicated access point.

#####Myriad 2 API:

#####Answer to the client:
The Myriad 2 must send a message to the client in the same topic indicating the successful creation of the access point or that the device has not been able to create it.

<a name="9"/>
###9. Connect to Access Point
#####Parameters:
* Ssid: SSID of the network to be connected to.
* Security: Security of the network. The values of this parameter must be Open, WEP or WPA.
* Pass: Password of the WiFi network if necessary (only on WEP and WPA networks).

#####Functionality:
Connect to the indicated access point (if it exists).

#####Myriad 2 API:

#####Answer to the client:
The Myriad 2 must send a message to the client in the same topic indicating the successful connection or that the device has not been able to connect to the access point.

<a name="10"/>
###10. Disconnect from Access Point
#####Parameters:
No parameters.

#####Functionality:
Disconnect from the current connected access point (if any).

#####Myriad 2 API:

#####Answer to the client:
The Myriad 2 must send a message to the client in the same topic indicating the successful disconnection or that the device is not connected to any access point.

<a name="11"/>
###11. List WiFi networks
#####Parameters:
No parameters.

#####Functionality:
Send to the client a list with the name and security type of the WiFi networks the device is able to connect.

#####Myriad 2 API:

#####Answer to the client:
The Myriad 2 must send a message to the client in the same topic indicating the list of WiFis.

<a name="12"/>
###12. Update date
#####Parameters:
* Year: Year to be updated.
* Month: Month to be updated.
* Day: Day to be updated.
* Hour: Hour to be updated.
* Min: Minute to be updated.
* Sec: Second to be updated.

#####Functionality:
Update the date of the device.

#####Myriad 2 API:

#####Answer to the client:
The Myriad 2 must send a message to the client in the same topic indicating that the date has been updated correctly.


##API functions on Myriad site

<a name="receiveFile"/>
###1. Receive File

unsigned char** receive_file(int number_of_packets, int payload_max, int chars_number_of_packet, unsigned int s, int* payload_image_size, unsigned char* dup2, int* qos2, unsigned char* retained2, unsigned short* packetid2, MQTTString* topicName2)

This function returns a set of pointers unsigned char** which corresponds to the file. Each pointer contains a package of 1KB maximum of the received file in order. The rest of parameters are:

* [in] number_of_packets - number of packets to receive
* [in] payload_max - max size of the payload of each part of the file (1024 bytes)
* [in] chars_number_of_packet - number of chars used at the end of the payload for indicating the number of the packet (6)
* [in] s - socket used for communication
* [out] payload_image_size - complete size of the file/picture
* [out] dup2 - dup parameter for MQTT in last message
* [out] qos2 - qos parameter for MQTT in last message
* [out] retained2 - retained parameter for MQTT in last message
* [out] packetid2 - packetid parameter for MQTT in last message
* [out] topicName2 - topicName parameter for MQTT in last message

<a name="sendFile"/>

###2. Send File

int send_file(pulgamqttbroker *pulga, topic_subscriptions *aux_t, unsigned int s, int sleep_packages, unsigned char** payload_image, int number_of_packets, int payload_max, int payload_image_size, unsigned char dup2, int qos2, unsigned char retained2, unsigned short packetid2, MQTTString topicName2)

This function send a file, indicating the number of packets, characterized by a set of pointers unsigned char**. Each pointer contains a package of 1KB maximum of the received file in order. It returns 1 if the file has been sent correctly. The rest of parameters are:

* [in] pulga - MQTT broker
* [in] aux_t - subscribers subscribed to the topic
* [in] s - socket used for communication
* [in] sleep_packages - o if not socket sleep implemented, 1 if it is implemented
* [in] payload_image - file to be sent
* [in] number_of_packets - number of packets to be sent
* [in] payload_max - max size of the payload of each part of the file (1024 bytes)
* [in] payload_image_size - complete size of the file/picture
* [in] dup2 - dup parameter for MQTT
* [in] qos2 - qos parameter for MQTT
* [in] retained2 - retained parameter for MQTT
* [in] packetid2 - packetid parameter for MQTT
* [in] topicName2 - topicName parameter for MQTT