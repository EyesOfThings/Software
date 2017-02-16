# RTEMS_API_REST_example

This application connects to a server previously created in the cloud (https://www.pythonanywhere.com, which comes with OpenCV 2.4 pre-installed). The sample application sends a POST request to the REST API of the server, including a picture which is read from the SD card. The server example returns its width and height in pixels. Consult the associated documentation for the task for more details on the server side.

The data of the access point must be changed in line 105:
`connectToAP("AndroidAP", "tqwg4654", SL_SEC_TYPE_WPA_WPA2, 20);`

The picture 0.9kb.jpg must be copied to the SD card (/mnt/sdcard/).
