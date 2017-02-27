package eu.eyesofthings.googleplayskeletonapp;



import org.eclipse.paho.android.service.MqttAndroidClient;
import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.util.Arrays;
import java.util.Calendar;
import java.util.GregorianCalendar;

import android.support.annotation.Nullable;
import android.content.Context;
import android.util.Log;

public class MQTT_Client implements MqttCallback {

    public interface OnErrorListener {
        public void onError(String message);
    }
    public interface OnConnectionLostListener {
        public void onConnectionLost(Throwable cause);
    }
    public interface OnResultListener {
        public void onSuccess(@Nullable Object result);
        public void onFailure(@Nullable Object result);
    }
    public interface OnMessageListener {
        public void onIncomingMessage(String topic, String message);
    }


    private interface OnReceivedListener {
        public void onReceivedData(Object data);
    }


    private static final String TCP_PROTOCOL = "tcp";
    private static final boolean CLEAN_SESSION = true;
    public static final String TOPIC_SNAPSHOT = "EOTSnapshot";
    public static final String TOPIC_EOT_CONNECT_TO_AP = "EOTConnectToAP";
    public static final String TOPIC_EOT_DISCONNECT_FROM_AP = "EOTDisconnectFromAP";
    public static final String TOPIC_EOT_CREATE_AP = "EOTCreateAP";
    public static final String TOPIC_EOT_DOWNLOAD_FILE_SD = "EOTDownloadFileSD";
    public static final String TOPIC_EOT_UPLOAD_FILE_SD = "EOTUploadFileSD";
    public static final String TOPIC_EOT_UPLOAD_ELF = "EOTUploadElf";
    public static final String TOPIC_EOT_UPDATE_DATE = "EOTUpdateDate";
    public static final String TOPIC_EOT_GET_DATE = "EOTGetDate";
    public static final String TOPIC_EOT_LIST_FILES_SD = "EOTListFilesSD";
    public static final String TOPIC_EOT_DELETE_FILE_SD = "EOTDeleteFileSD";
    public static final String TOPIC_EOT_DELETE_DIR_SD = "EOTDeleteDirSD";
    public static final String TOPIC_EOT_DELETE_CONTENT_SD = "EOTDeleteContentSD";
    public static final String TOPIC_EOT_MAKE_DIR_SD = "EOTMakeDirSD";

    private static final String MESSAGE = "EOT - MQTT Client";

    OnErrorListener mErrorCallback;
    OnConnectionLostListener mConnectionLostCallback;
    OnMessageListener mMessageCallback;
    OnReceivedListener mReceivedCallback;

    private String m_brokerIP;
    private int m_brokerPort;
    private String m_brokerURI;
    private int m_qualityOfService = 0;

    MqttConnectOptions m_connectionOptions;
    MqttAndroidClient m_client;

    private boolean m_firstPackage = true;
    private int m_fileLength = 0; // contains not really the size of a file, it contains the number of packets -> rename it to m_numberOfPackage
    private int m_actualPacket = 0; // rename it to m_currentPackage
    private byte[] m_fileData = null;

    private int m_length_sent = 0;
    private int m_actual_packet_sent = 0;
    private byte[]  m_upload_data;
    private int m_total_length = 1024;

    private String m_paths[];




    private volatile boolean m_operationDone;

    public MQTT_Client(Context context, String brokerIP, int brokerPort) throws MqttException {
        m_brokerIP = brokerIP;
        m_brokerPort = brokerPort;
        m_brokerURI = String.format("%s://%s:%d", MQTT_Client.TCP_PROTOCOL, m_brokerIP, m_brokerPort);
        //
        // Construct the connection options object
        //
        m_connectionOptions = new MqttConnectOptions();
        m_connectionOptions.setCleanSession(MQTT_Client.CLEAN_SESSION);
        m_connectionOptions.setKeepAliveInterval(0);
        //
        // Construct an MQTTClientThread blocking mode client
        m_client = new MqttAndroidClient(context, m_brokerURI, MqttClient.generateClientId());
        // Set this wrapper as the callback handler
        m_client.setCallback(this);
    }

    public void connect(final OnResultListener callback) throws MqttException, InterruptedException {
        if( !m_client.isConnected() ) {
            IMqttToken token = m_client.connect(m_connectionOptions, null, new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    callback.onSuccess(null);
                }
                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    callback.onFailure(exception.getMessage());
                }
            });
        }
    }

    public void disconnect(final OnResultListener callback){
        if(m_client.isConnected()){
            try {
                m_client.disconnect(null, new IMqttActionListener() {
                    @Override
                    public void onSuccess(IMqttToken asyncActionToken) {
                        callback.onSuccess(null);
                    }
                    @Override
                    public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                        callback.onFailure(exception.getMessage());
                    }
                });
            }
            catch (MqttException e) {
                callback.onFailure(e.getMessage());
            }
        }
    }

    public void publish(String topicName, int qos, byte[] payload, final OnResultListener callback) {
        // Create and configure a message
        MqttMessage message = new MqttMessage(payload);
        message.setQos(qos);

        // Send the message to the server, control is not returned until
        // it has been delivered to the server meeting the specified
        // quality of service.
        try {
            m_client.publish(topicName, message, null, new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    callback.onSuccess(null);
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    callback.onFailure(exception.getMessage());
                }
            });
        }
        catch (MqttException e) {
            callback.onFailure(e.getMessage());
        }

    }

    public void subscribe(String topicName, int qos, final OnResultListener callback) {
        try {
            m_client.subscribe(topicName, qos, null, new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    callback.onSuccess(null);
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    callback.onFailure(exception.getMessage());
                }
            });
        }
        catch (MqttException e) {
            callback.onFailure(e.getMessage());
        }
    }

    public void unsubscribe(String topicName, final OnResultListener callback) {
        try {
            m_client.unsubscribe(topicName, null, new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    callback.onSuccess(null);
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    callback.onFailure(exception.getMessage());
                }
            });
        }
        catch (MqttException e) {
            callback.onFailure(e.getMessage());
        }
    }

    public void askSnapshot(final OnResultListener callback) {
        // Ask for the snapshot
        subscribe(MQTT_Client.TOPIC_SNAPSHOT, m_qualityOfService, new OnResultListener() {
            @Override
            public void onSuccess(@Nullable Object result) {
                mReceivedCallback = new OnReceivedListener() {
                    @Override
                    public void onReceivedData(Object data) {
                        callback.onSuccess(data); // data: byte[]
                        mReceivedCallback = null;
                    }
                };
                publish(MQTT_Client.TOPIC_SNAPSHOT, m_qualityOfService, MQTT_Client.MESSAGE.getBytes(), new OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                        callback.onSuccess(result);
                    }
                    @Override
                    public void onFailure(@Nullable Object result) {
                        callback.onFailure(result);
                    }
                });
            }
            @Override
            public void onFailure(@Nullable Object result) {
                callback.onFailure(result);
            }
        });
    }

    private void uploadFileRecursive(final OnResultListener callback) {
        if(m_length_sent < m_upload_data.length) {
            //Thread.sleep(sleep_time); //sleep 0.1 sec (10KB/s)
            //add number of package (6 characters)
            m_actual_packet_sent = m_length_sent / m_total_length;
            String number_6zeros = String.format("%06d", m_actual_packet_sent);

            //create message (last 6 characters are the number of the package)
            byte[] subData1 = Arrays.copyOfRange(m_upload_data, m_length_sent, Math.min(m_length_sent + m_total_length, m_upload_data.length));
            byte[] subData2 = number_6zeros.getBytes();

            byte[] subData = new byte[subData1.length + subData2.length];
            System.arraycopy(subData1, 0, subData, 0, subData1.length);
            System.arraycopy(subData2, 0, subData, subData1.length, subData2.length);
            MqttMessage message1;
            message1 = new MqttMessage(subData);
            message1.setQos(m_qualityOfService);
            try {
                m_client.publish(TOPIC_EOT_UPLOAD_FILE_SD, message1, null, new IMqttActionListener() {
                    @Override
                    public void onSuccess(IMqttToken asyncActionToken) {
                        m_length_sent += m_total_length;
//                        uploadFileRecursive(callback);
                    }
                    @Override
                    public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                        callback.onFailure(exception.getMessage());
                    }
                });
            }
            catch (MqttException e) {
                callback.onFailure(e.getMessage());
            }
        }
    }
    public void uploadFile(final BufferedInputStream fileStream, final long fileSize, final String dstName, final OnResultListener callback) {
        subscribe(MQTT_Client.TOPIC_EOT_UPLOAD_FILE_SD, m_qualityOfService, new OnResultListener() {
            @Override
            public void onSuccess(@Nullable Object result) {
                try {
                    m_upload_data = new byte[(int) fileSize];
                    fileStream.read(m_upload_data);
                    fileStream.close();
                }
                catch (Exception e) {
                    callback.onFailure(e.getMessage());
                    return;
                }
                //add 1024 char limit to payload
                m_total_length = 1024;
                //message 1 (buffer length)
                int numberOfPackage = m_upload_data.length / m_total_length;
                if( m_upload_data.length % m_total_length != 0) {
                    ++numberOfPackage;
                }
                MqttMessage message2 = new MqttMessage((Integer.toString(numberOfPackage) + " " + dstName).getBytes());
                message2.setQos(m_qualityOfService);

                try {
                    mReceivedCallback = new OnReceivedListener() {
                        @Override
                        public void onReceivedData(Object asyncData) {
                        Integer receivedAnswer = (Integer) asyncData;
                        if(receivedAnswer.intValue() == 2) {
                            uploadFileRecursive(callback);
                        }
                        else {
                            if(receivedAnswer.intValue() == 0) {
                                callback.onSuccess(asyncData);
                            }
                            else {
                                callback.onFailure(asyncData);
                            }
                            mReceivedCallback = null;
                        }
                        }
                    };
                    m_client.publish(MQTT_Client.TOPIC_EOT_UPLOAD_FILE_SD, message2, null, new IMqttActionListener() {
                        @Override
                        public void onSuccess(final IMqttToken asyncActionToken) {
                            m_length_sent = 0;
                            m_actual_packet_sent = 0;
                            uploadFileRecursive(callback);
                        }
                        @Override
                        public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                            callback.onFailure(exception.getMessage());
                        }
                    });
                }
                catch (MqttException e) {
                    callback.onFailure(e.getMessage());
                }
            }
            @Override
            public void onFailure(@Nullable Object result) {
                callback.onFailure(result);
            }
        });
    }
    public void uploadFile(final String srcDir,  final String dstName, final OnResultListener callback) {
        File sourceFile = new File( srcDir );
        try {
            BufferedInputStream fileStream = new BufferedInputStream(new FileInputStream(sourceFile));
            uploadFile(fileStream, sourceFile.length(), dstName, callback);
        }
        catch (Exception e) {
            callback.onFailure(e.getMessage());
            return;
        }
    }

    public void downloadFile(final String srcDir, final OnResultListener callback) {
        subscribe(MQTT_Client.TOPIC_EOT_DOWNLOAD_FILE_SD, m_qualityOfService, new OnResultListener() {
            @Override
            public void onSuccess(@Nullable Object result) {
                mReceivedCallback = new OnReceivedListener() {
                    @Override
                    public void onReceivedData(Object data) {
                        callback.onSuccess(data);
                        mReceivedCallback = null;
                    }
                };
                publish(MQTT_Client.TOPIC_EOT_DOWNLOAD_FILE_SD, m_qualityOfService, srcDir.getBytes(), new OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                    }

                    @Override
                    public void onFailure(@Nullable Object result) {
                        callback.onFailure(result);
                    }
                });
            }
            @Override
            public void onFailure(@Nullable Object result) {
                callback.onFailure(result);
            }
        });
    }

    public boolean isConnected(){
        return m_client.isConnected();
    }

    public void getFileSystemStructure(final String path, final OnResultListener callback) {
        m_firstPackage = true;
        subscribe(MQTT_Client.TOPIC_EOT_LIST_FILES_SD, m_qualityOfService, new OnResultListener() {
            @Override
            public void onSuccess(@Nullable Object result) {
                mReceivedCallback = new OnReceivedListener() {
                    @Override
                    public void onReceivedData(Object data) {
                        callback.onSuccess(data);
                        mReceivedCallback = null;
                    }
                };
                publish(MQTT_Client.TOPIC_EOT_LIST_FILES_SD, m_qualityOfService, path.getBytes(), new OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                    }
                    @Override
                    public void onFailure(@Nullable Object result) {
                        callback.onFailure(result);
                    }
                });
            }
            @Override
            public void onFailure(@Nullable Object result) {
                callback.onFailure(result);
            }
        });
    }

    public void createFolder(final String path,final OnResultListener callback) {
        subscribe(MQTT_Client.TOPIC_EOT_MAKE_DIR_SD, m_qualityOfService, new OnResultListener() {
            @Override
            public void onSuccess(@Nullable Object result) {
                mReceivedCallback = new OnReceivedListener() {
                    @Override
                    public void onReceivedData(Object data) {
                        callback.onSuccess(data);
                        mReceivedCallback = null;
                    }
                };
                publish(MQTT_Client.TOPIC_EOT_MAKE_DIR_SD, m_qualityOfService, path.getBytes(), new OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                    }
                    @Override
                    public void onFailure(@Nullable Object result) {
                        callback.onFailure(result);
                    }
                });
            }
            @Override
            public void onFailure(@Nullable Object result) {
                callback.onFailure(result);
            }
        });
    }


    public void removeFile(final String path, final OnResultListener callback) {
        subscribe(MQTT_Client.TOPIC_EOT_DELETE_FILE_SD, m_qualityOfService, new OnResultListener() {
            @Override
            public void onSuccess(@Nullable Object result) {
                mReceivedCallback = new OnReceivedListener() {
                    @Override
                    public void onReceivedData(Object data) {
                        callback.onSuccess(data);
                        mReceivedCallback = null;
                    }
                };
                publish(MQTT_Client.TOPIC_EOT_DELETE_FILE_SD, m_qualityOfService, path.getBytes(), new OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                    }

                    @Override
                    public void onFailure(@Nullable Object result) {
                        callback.onFailure(result);
                    }
                });
            }
            @Override
            public void onFailure(@Nullable Object result) {
                callback.onFailure(result);
            }
        });
    }

    public void removeContent(final String path, final OnResultListener callback) {
        subscribe(MQTT_Client.TOPIC_EOT_DELETE_CONTENT_SD, m_qualityOfService, new OnResultListener() {
            @Override
            public void onSuccess(@Nullable Object result) {
                mReceivedCallback = new OnReceivedListener() {
                    @Override
                    public void onReceivedData(Object data) {
                        callback.onSuccess(data);
                        mReceivedCallback = null;
                    }
                };
                publish(MQTT_Client.TOPIC_EOT_DELETE_CONTENT_SD, m_qualityOfService, path.getBytes(), new OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                    }

                    @Override
                    public void onFailure(@Nullable Object result) {
                        callback.onFailure(result);
                    }
                });
            }

            @Override
            public void onFailure(@Nullable Object result) {
                callback.onFailure(result);
            }
        });
    }

    public void removeAll(final String path, final OnResultListener callback ) {
        subscribe(MQTT_Client.TOPIC_EOT_DELETE_DIR_SD, m_qualityOfService, new OnResultListener() {
            @Override
            public void onSuccess(@Nullable Object result) {
                mReceivedCallback = new OnReceivedListener() {
                    @Override
                    public void onReceivedData(Object data) {
                        callback.onSuccess(data);
                        mReceivedCallback = null;
                    }
                };
                publish(MQTT_Client.TOPIC_EOT_DELETE_DIR_SD, m_qualityOfService, path.getBytes(), new OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                    }
                    @Override
                    public void onFailure(@Nullable Object result) {
                        callback.onFailure(result);
                    }
                });
            }

            @Override
            public void onFailure(@Nullable Object result) {
                callback.onFailure(result);
            }
        });
    }


    private void uploadProgramRecursive(final OnResultListener callback) {
        if(m_length_sent < m_upload_data.length) {
            //add number of package (6 characters)
            m_actual_packet_sent = m_length_sent / m_total_length;
            String number_6zeros = String.format("%06d", m_actual_packet_sent);

            //create message (last 6 characters are the number of the package)
            byte[] subData1 = Arrays.copyOfRange(m_upload_data, m_length_sent, Math.min(m_length_sent + m_total_length, m_upload_data.length));
            byte[] subData2 = number_6zeros.getBytes();

            byte[] subData = new byte[subData1.length + subData2.length];
            System.arraycopy(subData1, 0, subData, 0, subData1.length);
            System.arraycopy(subData2, 0, subData, subData1.length, subData2.length);
            MqttMessage message1;
            message1 = new MqttMessage(subData);
            message1.setQos(m_qualityOfService);
            try {
                m_client.publish(MQTT_Client.TOPIC_EOT_UPLOAD_ELF, message1, null, new IMqttActionListener() {
                    @Override
                    public void onSuccess(IMqttToken asyncActionToken) {
                        m_length_sent += m_total_length;
                    }
                    @Override
                    public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                        callback.onFailure(exception.getMessage());
                    }
                });
            }
            catch (MqttException e) {
                callback.onFailure(e.getMessage());
            }
        }
    }
    public void uploadProgram(final BufferedInputStream fileStream, final long fileSize, final OnResultListener callback) {
        subscribe(MQTT_Client.TOPIC_EOT_UPLOAD_ELF, m_qualityOfService, new OnResultListener() {
            @Override
            public void onSuccess(@Nullable Object result) {
                try {
                    m_upload_data = new byte[(int) fileSize];
                    fileStream.read(m_upload_data);
                    fileStream.close();
                }
                catch (Exception e) {
                    callback.onFailure(e.getMessage());
                    return;
                }
                //add 1024 char limit to payload
                m_total_length = 1024;
                //message 1 (buffer length)
                int numberOfPackage = m_upload_data.length / m_total_length;
                if( m_upload_data.length % m_total_length != 0) {
                    ++numberOfPackage;
                }
                MqttMessage message2 = new MqttMessage((Integer.toString(numberOfPackage) + " dummy.d").getBytes());
                message2.setQos(m_qualityOfService);
                try {
                    mReceivedCallback = new OnReceivedListener() {
                        @Override
                        public void onReceivedData(Object asyncData) {
                            Integer receivedAnswer = (Integer) asyncData;
                            if(receivedAnswer.intValue() == 2) {
                                uploadProgramRecursive(callback);
                            }
                            else {
                                if(receivedAnswer.intValue() == 0) {
                                    callback.onSuccess(asyncData);
                                }
                                else {
                                    callback.onFailure(asyncData);
                                }
                                mReceivedCallback = null;
                            }
                        }
                    };
                    m_client.publish(MQTT_Client.TOPIC_EOT_UPLOAD_ELF, message2, null, new IMqttActionListener() {
                        @Override
                        public void onSuccess(final IMqttToken asyncActionToken) {
                            m_length_sent = 0;
                            m_actual_packet_sent = 0;
                            uploadProgramRecursive(callback);
                        }
                        @Override
                        public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                            callback.onFailure(exception.getMessage());
                        }
                    });
                }
                catch (MqttException e) {
                    callback.onFailure(e.getMessage());
                }
            }
            @Override
            public void onFailure(@Nullable Object result) {
                callback.onFailure(result);
            }
        });
    }
    public void uploadProgram(final String srcDir, final OnResultListener callback) {
        File sourceFile = new File( srcDir );
        try {
            BufferedInputStream fileStream = new BufferedInputStream(new FileInputStream(sourceFile));
            uploadProgram(fileStream, sourceFile.length(), callback);
        }
        catch (Exception e) {
            callback.onFailure(e.getMessage());
            return;
        }
    }



    public void resetAPConfig(final OnResultListener callback) {
        subscribe(MQTT_Client.TOPIC_EOT_DISCONNECT_FROM_AP, m_qualityOfService, new OnResultListener() {
            @Override
            public void onSuccess(@Nullable Object result) {
                publish(MQTT_Client.TOPIC_EOT_DISCONNECT_FROM_AP, m_qualityOfService, ("").getBytes(), callback);
            }
            @Override
            public void onFailure(@Nullable Object result) {
                callback.onFailure(result);
            }
        });
    }

    public void createAP(final String SSID, final String security, final String pass, final String channel, final OnResultListener callback)  {
        subscribe(MQTT_Client.TOPIC_EOT_CREATE_AP, m_qualityOfService, new OnResultListener() {
            @Override
            public void onSuccess(@Nullable Object result) {
                publish(MQTT_Client.TOPIC_EOT_CREATE_AP, m_qualityOfService, (SSID + " " + security + " " + pass + " " + channel).getBytes(), callback);
            }
            @Override
            public void onFailure(@Nullable Object result) {
                callback.onFailure(result);
            }
        });
    }
    public void connectToAP(final String SSID, final String security, final String pass, final OnResultListener callback) {
        subscribe(MQTT_Client.TOPIC_EOT_CONNECT_TO_AP, m_qualityOfService, new OnResultListener() {
            @Override
            public void onSuccess(@Nullable Object result) {
                publish(MQTT_Client.TOPIC_EOT_CONNECT_TO_AP, m_qualityOfService, (SSID + " " + security + " " + pass).getBytes(), callback);
            }
            @Override
            public void onFailure(@Nullable Object result) {
                callback.onFailure(result);
            }
        });
    }

    public void updateDate(final String year, final String month, final String day, final String hour, final String mins, final String secs, final OnResultListener callback) {
        subscribe(MQTT_Client.TOPIC_EOT_UPDATE_DATE, m_qualityOfService, new OnResultListener() {
            @Override
            public void onSuccess(@Nullable Object result) {
                String date = year + " "
                        + month + " "
                        + day + " "
                        + hour + " "
                        + mins + " "
                        + secs;
                mReceivedCallback = new OnReceivedListener() {
                    @Override
                    public void onReceivedData(Object data) {
                        callback.onSuccess(data);
                        mReceivedCallback = null;
                    }
                };
                publish(MQTT_Client.TOPIC_EOT_UPDATE_DATE, m_qualityOfService, date.getBytes(), new OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                    }

                    @Override
                    public void onFailure(@Nullable Object result) {
                        callback.onFailure(result);
                    }
                });
            }
            @Override
            public void onFailure(@Nullable Object result) {
                callback.onFailure(result);
            }
        });
    }
    public void getDate(final OnResultListener callback) {
        subscribe(MQTT_Client.TOPIC_EOT_GET_DATE, m_qualityOfService, new OnResultListener() {
            @Override
            public void onSuccess(@Nullable Object result) {
                mReceivedCallback = new OnReceivedListener() {
                    @Override
                    public void onReceivedData(Object asyncData) {
                        String currentDeviceDate = (String) asyncData;
                        Calendar curDevDate = new GregorianCalendar();
                        String[] data = currentDeviceDate.split("\\s+");
                        int year = Integer.parseInt(data[4]);
                        int day = Integer.parseInt(data[2]);
                        int hourOfDay = Integer.parseInt(data[3].substring(0, 2));
                        int minute = Integer.parseInt(data[3].substring(3, 5));
                        int second = Integer.parseInt(data[3].substring(6));
                        int month = 0;
                        switch (data[1]) {
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
                        curDevDate.set(year, month, day, hourOfDay, minute, second);

                        callback.onSuccess(curDevDate);
                        mReceivedCallback = null;
                    }
                };
                publish(MQTT_Client.TOPIC_EOT_GET_DATE, m_qualityOfService, ("").getBytes(), new OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                    }

                    @Override
                    public void onFailure(@Nullable Object result) {
                        callback.onFailure(result);
                    }
                });
            }

            @Override
            public void onFailure(@Nullable Object result) {
                callback.onFailure(result);
            }
        });
    }


    @Override
    public void connectionLost(Throwable cause) {
//        Log.v("connectionLost", "Connection to " + m_brokerURI + " lost!" + cause);
        if(null != mConnectionLostCallback) {
            mConnectionLostCallback.onConnectionLost(cause);
        }
    }

    @Override
    public void messageArrived(String topic, MqttMessage message) throws Exception {
        // Called when a message arrives from the server that matches any
        // subscription made by the client
        switch (topic) {
            case MQTT_Client.TOPIC_SNAPSHOT:
                // Receives part of the snapshot
                if (m_firstPackage == true) {
                    m_fileLength = Integer.parseInt(new String(message.getPayload()));
                    m_firstPackage = false;
                } else {
                    if (m_actualPacket == 0) {
                        m_fileData = message.getPayload();
                    } else {
                        byte[] b = message.getPayload();
                        byte[] c = new byte[m_fileData.length + b.length];
                        System.arraycopy(m_fileData, 0, c, 0, m_fileData.length);
                        System.arraycopy(b, 0, c, m_fileData.length, b.length);
                        m_fileData = c;
                    }
                    m_actualPacket++;
                }
                if (m_actualPacket == m_fileLength) {
                    m_actualPacket = 0;
                    m_firstPackage = true;
                    if (null != mReceivedCallback) {
                        mReceivedCallback.onReceivedData(m_fileData);
                    }
                }
                break;
            case MQTT_Client.TOPIC_EOT_DOWNLOAD_FILE_SD:
                // Receives part of the file
                if (m_firstPackage == true) {
                    m_fileLength = Integer.parseInt(new String(message.getPayload()));
                    m_firstPackage = false;
                } else {
                    if (m_actualPacket == 0) {
                        m_fileData = message.getPayload();
                    } else {
                        byte[] b = message.getPayload();
                        byte[] c = new byte[m_fileData.length + b.length];
                        System.arraycopy(m_fileData, 0, c, 0, m_fileData.length);
                        System.arraycopy(b, 0, c, m_fileData.length, b.length);
                        m_fileData = c;
                    }
                    m_actualPacket++;
                }
                if (m_actualPacket == m_fileLength) {
                    m_actualPacket = 0;
                    m_firstPackage = true;
                    if (null != mReceivedCallback) {
                        mReceivedCallback.onReceivedData(m_fileData);
                    }
                }
                break;
            case MQTT_Client.TOPIC_EOT_UPLOAD_FILE_SD:
                // Receives the result of uploading a file to the SD card
                int fileUploadResult = Integer.parseInt(new String(message.getPayload()));
                if (null != mReceivedCallback) {
                    mReceivedCallback.onReceivedData( fileUploadResult );
                }
                break;
            case MQTT_Client.TOPIC_EOT_GET_DATE:
                // Receives the device date
                String currentDeviceDate = new String(message.getPayload());
                if (null != mReceivedCallback) {
                    mReceivedCallback.onReceivedData(currentDeviceDate);
                }
                break;
            case MQTT_Client.TOPIC_EOT_UPDATE_DATE:
                // Receives the result of updating the date
                int updateDateResult = Integer.parseInt(new String(message.getPayload()));
                if (null != mReceivedCallback) {
                    mReceivedCallback.onReceivedData( updateDateResult );
                }
                break;
            case MQTT_Client.TOPIC_EOT_UPLOAD_ELF:
                // Receives the result of uploading an elf file to the flash memmory
                Integer uploadElfResult = Integer.parseInt(new String(message.getPayload()));
                if (null != mReceivedCallback) {
                    mReceivedCallback.onReceivedData(uploadElfResult);
                }
                break;
            case MQTT_Client.TOPIC_EOT_LIST_FILES_SD:
                // Receives part of the SD card files paths
                if (m_firstPackage == true) {
                    int packs = Integer.parseInt(new String(message.getPayload()));
                    m_fileLength = packs;
                    m_paths = new String[packs];
                    m_firstPackage = false;
                    m_actualPacket = 0;
                } else {
                    m_paths[this.m_actualPacket] = new String(message.getPayload());
                    m_actualPacket++;
                }
                if (m_actualPacket == m_fileLength) {
                    m_actualPacket = 0;
                    m_firstPackage = true;
                    if (null != mReceivedCallback) {
                        mReceivedCallback.onReceivedData(m_paths);
                    }
                }
                break;
            case MQTT_Client.TOPIC_EOT_MAKE_DIR_SD:
            case MQTT_Client.TOPIC_EOT_DELETE_FILE_SD:
            case MQTT_Client.TOPIC_EOT_DELETE_CONTENT_SD:
            case MQTT_Client.TOPIC_EOT_DELETE_DIR_SD:
                int changedSDCardResult = Integer.parseInt(new String(message.getPayload()));
                if (null != mReceivedCallback) {
                    mReceivedCallback.onReceivedData(changedSDCardResult);
                }
                break;
            default:
                if(null != mMessageCallback) {
                    mMessageCallback.onIncomingMessage(topic, message.toString());
                }
                break;
        }
    }




    @Override
    public void deliveryComplete(IMqttDeliveryToken token) {
//        Log.v("deliveryComplete", "bam");
    }

    public void setOnErrorListener(OnErrorListener listener) {
        mErrorCallback = listener;
    }

    public void setOnConnectionLostListener(OnConnectionLostListener listener) {
        mConnectionLostCallback = listener;
    }
    public void setOnMessageListener(OnMessageListener listener) {
        mMessageCallback = listener;
    }
}
