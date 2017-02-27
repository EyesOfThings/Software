/*!
    \file
    \brief     Pulga MQTT Broker Control library for Eyes of Things Project
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <time.h>
#include <unistd.h>
#include <rtems.h>
#include "eot/image.h"

/*
//utime for change creation and modification time of the created file
#include <utime.h>
#include <sys/stat.h>
#include <errno.h>
 */

#include "DrvTimer.h"
#include "simplelink.h"
#include "PulgaMQTTBrokerControl.h"
#include "condStates.h"

condState state = EXECUTE_PYTHON;
pthread_cond_t pythonTurn = PTHREAD_COND_INITIALIZER;
pthread_cond_t pulgaTurn = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;



//Micropython variables
MQTTString topicShowImage;
unsigned short showImagepacketid;
char buff_micropython[10240];
static char *stack_top;
static char heap[1000000];
//for sending shows
//extern unsigned char** image_to_show_buffer;
//extern int images_saved;



/*mp_obj_t callback_print(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
	const char *str = mp_obj_str_get_str(args[0]);
	printf("c_print: %s\n", str);

	return mp_const_none;
}*/

static mp_obj_t callback_print(void)
{
	//const char *str = mp_obj_str_get_str(args[0]);
	printf("c_print:\n");

	return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_0(pyb_lcd_init_obj, callback_print);

/*!
 *  \brief Create empty Pulga MQTT broker
 *  \return         Broker created
*/
pulgamqttbroker *pulga_create() {
    pulgamqttbroker *pulga = malloc(sizeof (pulgamqttbroker));
    if (pulga == NULL) {
        fprintf(stderr, "Error: could not allocate memory for Pulga MQTT broker\n");
        exit(EXIT_FAILURE);
    }
    pulga->name = "PulgaMqttBrokerControl";
    pulga->connected_clients = ll_create();
    pulga->topics_subscribers = ll_create();
    
    //start micropython
    int stack_dummy;
    stack_top = (char*) &stack_dummy;
    gc_init(heap, heap + sizeof (heap));
    mp_init();
    
    //mp_store_name(qstr_from_str("c_print"), mp_obj_new_fun_native(0, 1, 0, (mp_obj_t)&pyb_lcd_init_obj));
    mp_store_name(qstr_from_str("c_print"), (mp_obj_t)&pyb_lcd_init_obj);
    //image_to_show_buffer=(unsigned char**) calloc(100, sizeof(char*));
    
    return pulga; 
}

/*!
 *  \brief Create new topic for subscription without subscribed clients
 *  \param[in]      topid id
 *  \return         topic subscriptions created
*/
topic_subscriptions *create_topic_subscription(char* c) {
    topic_subscriptions *topic = malloc(sizeof (topic_subscriptions));
    if (topic == NULL) {
        fprintf(stderr, "Error: could not allocate memory for topic\n");
        exit(EXIT_FAILURE);
    }
    strcpy(topic->id, c);
    topic->queued_messages = ll_create();
    topic->suscribed = ll_create();
    return topic;
}

/*!
 *  \brief Create new topic for subscription with a subscribed client
 *  \param[in]      topid id
 *  \param[in]      client subscribed
 *  \return         topic subscriptions created
*/
topic_subscriptions *create_topic_subscription1subscriber(char* c, int subscriber) {
    topic_subscriptions *topic = malloc(sizeof (topic_subscriptions));
    if (topic == NULL) {
        fprintf(stderr, "Error: could not allocate memory for topic\n");
        exit(EXIT_FAILURE);
    }
    strcpy(topic->id, c);
    topic->queued_messages = ll_create();
    topic->suscribed = ll_create();
    ll_push_last(topic->suscribed, new_int(subscriber));
    return topic;
}

/*!
 *  \brief Start the Pulga MQTT Broker
 *  \param[in]      Broker data previously created
 *  \return         0
*/
int pulga_start(pulgamqttbroker *pulga) {

    prepare_camera();
    //restarting parameter
    int restart=0;
    while (1) {

        //if restart parameter, create pulga again
        if(restart==1){
            reconfigure_camera();
            pulga = pulga_create();
        }
        
        SlFdSet_t socks;
        SlFdSet_t readsocks;
        int sock;
        int maxsock;
        int reuseaddr = 1; /* True */
        //new
        //struct addrinfo hints, *res;
        SlSockAddrIn_t LocalAddr;
        SlSockAddrIn_t their_addr;
        _i32 ServerSockID = -1;
        long nonBlocking;
        int AddrSize;
        unsigned char RecvBuf[BUF_SIZE];
        unsigned char SendBuf[BUF_SIZE];
        int len = 0;
        //MQTTTransport mytransport;
        int frc;
        int rc = 0;
        FILE *output = stdout;
        int errores_frc = 0;


        printf("Starting Pulga MQTT Broker: %s \n", pulga->name);


        /* Get the address info */
        LocalAddr.sin_family = SL_AF_INET;
        LocalAddr.sin_port = sl_Htons(PORT_NUM);
        LocalAddr.sin_addr.s_addr = 0;

        /* Create the socket */
        sock = sl_Socket(SL_AF_INET, SL_SOCK_STREAM, 0);
        if (sock == -1) {
            perror("socket");
            return 1;
        }

        /* Bind to the address */
        if (sl_Bind(sock, (SlSockAddr_t *) & LocalAddr, sizeof (SlSockAddrIn_t)) == -1) {
            perror("bind");
            return 1;
        }

        /* Listen */
        if (sl_Listen(sock, 0) == -1) {
            perror("listen");
            return 1;
        }


        /* Enable the socket */
        nonBlocking = 1;
        AddrSize = sizeof (SlSockAddrIn_t);
        if (sl_SetSockOpt(sock, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &nonBlocking, sizeof (nonBlocking)) == -1) {
            perror("setsockopt");
            return 1;
        }

        /* Set up the fd_set */
        SL_FD_ZERO(&socks);
        SL_FD_SET(sock, &socks);
        maxsock = sock;

        /* Main loop */
        restart = 0;
        while (restart == 0) {
            printCurrentTime();
            
            printf("Waiting\n");
            unsigned int s;
            //int sleep_milsec=1;
            readsocks = socks;
            //Sleep(sleep_milsec);
            if (sl_Select(maxsock + 1, &readsocks, NULL, NULL, NULL) == -1) {
                perror("select");
                return 1;
            }

            for (s = 0; s <= (unsigned int) maxsock; s++) {
                unsigned int numMs = 20;
                DrvTimerSleepMs(numMs);
                if (SL_FD_ISSET(s, &readsocks)) {
                    printf("socket %d was ready\n", s);
                    sl_SetSockOpt(s, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &nonBlocking, sizeof (nonBlocking));
                    if (s == (unsigned int) sock) {
                        /* New connection */
                        int newsock;
                        //Sleep(sleep_milsec);
                        printf("Accepting connection \n");
                        newsock = sl_Accept(sock, (struct SlSockAddr_t *) &their_addr, (SlSocklen_t*) & AddrSize);
                        if (newsock == -1) {
                            perror("accept");
                        } else {
                            printf("Got a connection on port %d\n", sl_Htons(their_addr.sin_port));
                            SL_FD_SET(newsock, &socks);
                            if (newsock > maxsock) {
                                maxsock = newsock;
                            }
                        }
                    } else {
                        /* Handle read or disconnection */
                        //handle(s, &socks);
                        printf("Handle message \n");
                        memset(RecvBuf, 0x00, sizeof (RecvBuf));
                        //Sleep(sleep_milsec);
                        frc = MQTTPacket_read2(RecvBuf, sizeof (RecvBuf), transport_getdata, s);
                        printf("************\n FRC value %d \n************ \n \n", frc);
                        if (frc == ERROR_FRC) { //if error packet
                            errores_frc++;
                            if (errores_frc >= ERROR_DISCONNECTION && ll_exists(pulga->connected_clients, new_int(s), compare_int)) {//incorrect disconnection
                                printf("Closing socket %d due to incorrect disconnection. \n", s);
                                SL_FD_CLR(s, &socks);
                                sl_Close(s);
                                ll_remove_item(pulga->connected_clients, new_int(s), compare_int);
                                errores_frc = 0;
                            }
                        } else {
                            errores_frc = 0;

                            //Connect message
                            if (frc == CONNECT) {
                                MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
                                MQTTPacket_connectData x;

                                //set socket as nonblocking
                                sl_SetSockOpt(s, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &nonBlocking, sizeof (nonBlocking));

                                rc = MQTTDeserialize_connect(&data, RecvBuf, sizeof (RecvBuf));
                                printf("Message received on socket %d\n", s);
                                printf("Message Received (ID of client): \n %d \n\n", data.clientID.lenstring.data);

                                //add to connected clients
                                //ll_push_last(pulga->connected_clients,&data);
                                ll_push_last(pulga->connected_clients, new_int(s));

                                //x = *((MQTTPacket_connectData *)ll_get_first(pulga->connected_clients));
                                //printf("Data 1st element: %s \n",x.clientID.lenstring.data);
                                printf("Connected clients socket: \n");
                                ll_print(pulga->connected_clients, output, to_string_print_int);

                                //CONACK ANSWER
                                memset(SendBuf, 0x00, sizeof (SendBuf));
                                len = MQTTSerialize_connack(SendBuf, sizeof (SendBuf), 0, 0);
                                //Sleep(sleep_milsec);
                                sl_Send(s, SendBuf, len, 0);
                            }

                            //Subscribe message
                            if (frc == SUBSCRIBE) {

                                unsigned char dup;
                                unsigned short packetid;
                                int maxcount = 1; //maximum number of members allowed
                                int count;
                                MQTTString topicfilters = MQTTString_initializer;
                                int requestedQoSs;
                                topic_subscriptions *aux = malloc(sizeof (topic_subscriptions));
                                int pos;
                                char str_copy[50];

                                rc = MQTTDeserialize_subscribe(&dup, &packetid, maxcount, &count, &topicfilters, &requestedQoSs, RecvBuf, sizeof (RecvBuf));

                                printf("Subscribe received to topic %s\n", topicfilters.lenstring.data);
                                printf("Subscribe received to topic (length topic) %d\n", topicfilters.lenstring);
                                printf("requestedQoSs %d\n", requestedQoSs);
                                printf("Count %d\n", count);
                                printf("RC %d\n", rc);

                                //subscribe the client to the topic
                                strncpy(str_copy, topicfilters.lenstring.data, topicfilters.lenstring.len);
                                str_copy[topicfilters.lenstring.len] = '\0';

                                aux = create_topic_subscription(str_copy);

                                pos = ll_item_position(pulga->topics_subscribers, aux, comparetopic_subscriptionsStruct);
                                printf("Initial pos of topic: %d \n", pos);
                                if (pos >= 0) { //add new subscriber to topic
                                    topic_subscriptions* aux_topic = (topic_subscriptions *) ll_get_index(pulga->topics_subscribers, pos);
                                    if (!ll_exists(aux_topic->suscribed, new_int(s), compare_int)) { //only if the subscriber is not added before
                                        ll_push_last(aux_topic->suscribed, new_int(s));
                                    }
                                } else { //create new topic including the first subscriber
                                    //ll_push_last(aux->suscribed,new_int(s));
                                    ll_push_last(pulga->topics_subscribers, create_topic_subscription1subscriber(str_copy, s));
                                }

                                //search the new added topic/client socket and print to see if it worked correctly
                                pos = ll_item_position(pulga->topics_subscribers, aux, comparetopic_subscriptionsStruct);
                                printf("Final pos of topic: %d \n", pos);
                                ll_print(((topic_subscriptions *) ll_get_index(pulga->topics_subscribers, pos))->suscribed, output, to_string_print_int);

                                //SUBACK ANSWER
                                memset(SendBuf, 0x00, sizeof (SendBuf));
                                len = MQTTSerialize_suback(SendBuf, sizeof (SendBuf), packetid, count, &requestedQoSs);
                                //Sleep(sleep_milsec);
                                sl_Send(s, SendBuf, len, 0);

                            }

                            //Unsubscribe message
                            if (frc == UNSUBSCRIBE) {

                                unsigned char dup;
                                unsigned short packetid;
                                int maxcount = 1; //maximum number of members allowed
                                int count;
                                MQTTString topicfilters = MQTTString_initializer;
                                topic_subscriptions *aux = malloc(sizeof (topic_subscriptions));
                                int pos;
                                char str_copy[50];

                                rc = MQTTDeserialize_unsubscribe(&dup, &packetid, maxcount, &count, &topicfilters, RecvBuf, sizeof (RecvBuf));

                                printf("Unsubscribe received to topic %s by socket %d \n", topicfilters.lenstring.data, s);

                                //unsubscribe the client to the topic
                                strncpy(str_copy, topicfilters.lenstring.data, topicfilters.lenstring.len);
                                str_copy[topicfilters.lenstring.len] = '\0';

                                aux = create_topic_subscription(str_copy);

                                pos = ll_item_position(pulga->topics_subscribers, aux, comparetopic_subscriptionsStruct);
                                printf("Pos of topic: %d \n", pos);
                                if (pos >= 0) { //remove subscriber from topic
                                    topic_subscriptions* aux_topic = (topic_subscriptions *) ll_get_index(pulga->topics_subscribers, pos);
                                    if (ll_remove_item(aux_topic->suscribed, new_int(s), compare_int)) { //remove subscriber if it exists
                                        printf("Socket %d unsubscribed from topic %s \n", s, topicfilters.lenstring.data);
                                    } else { //the topic doesn't contain the subscriber
                                        printf("Socket %d hasn't be subscribed before to topic %s \n", s, topicfilters.lenstring.data);
                                    }
                                    //show subscribers of the topic
                                    ll_print(((topic_subscriptions *) ll_get_index(pulga->topics_subscribers, pos))->suscribed, output, to_string_print_int);
                                } else { //topic doesn't exist
                                    printf("Topic %s doesn't exist \n", topicfilters.lenstring.data);
                                }

                                //UNSUBACK ANSWER
                                memset(SendBuf, 0x00, sizeof (SendBuf));
                                len = MQTTSerialize_unsuback(SendBuf, sizeof (SendBuf), packetid);
                                //Sleep(sleep_milsec);
                                sl_Send(s, SendBuf, len, 0);

                            }

                            //Publish message
                            if (frc == PUBLISH) {

                                unsigned char dup;
                                int qos;
                                unsigned char retained;
                                unsigned short packetid;
                                MQTTString topicName;
                                unsigned char* payload;
                                int payloadlen;
                                int pub_parser = -1;

                                rc = MQTTDeserialize_publish(&dup, &qos, &retained, &packetid, &topicName, &payload, &payloadlen, RecvBuf, sizeof (RecvBuf));
                                printf("Message received on topic %s: \n %s \n", topicName.lenstring.data, payload);

                                //PUBLISH TO SUSCRIBERS
                                pub_parser = publish_parser(pulga, s, dup, qos, retained, packetid, topicName, payload, payloadlen);
                                //restart if pub_parser<0
                                if (pub_parser < 0){
                                    restart=1;
                                }
                            }

                            //Pingreq message
                            if (frc == PINGREQ) {
                                printf("Answering ping request in socket %d. \n", s);
                                //PINGRESP ANSWER
                                memset(SendBuf, 0x00, sizeof (SendBuf));
                                len = MQTTSerialize_pingreq(SendBuf, sizeof (SendBuf));
                                //Sleep(sleep_milsec);
                                sl_Send(s, SendBuf, len, 0);
                            }

                            //disconnect message
                            if (frc == DISCONNECT) { //DELETE ALL THE INFO OF THE CLIENT
                                printf("Bye bye client . Closing socket %d. \n", s);
                                SL_FD_CLR(s, &socks);
                                sl_Close(s);
                                ll_remove_item(pulga->connected_clients, new_int(s), compare_int);
                            }
                        }

                    }
                }
            }

        }

        sl_Close(sock);

    }
    return 0;
}

/*!
 *  \brief Function for receiving data in a certain socket
 *  \param[out]     buffer with the data received
 *  \param[in]      specifies the length in bytes of the buffer pointed to by the buffer argument
 *  \param[in]      socket where receive the data
 *  \return         message rc of sl_Recv function
*/
int transport_getdata(unsigned char* buf, int count, int socket) {
    
    int rc = sl_Recv(socket, buf, count, 0);
    
    //printf("received %d bytes count %d\n", rc, (int)count);
    return rc;
}

//
/*!
 *  \brief Compare topic subscribed by name
 *  \param[in]      First topic to compare
 *  \param[in]      Second topic for compare
 *  \return         0 if the topics are the same, -1 if first is greater, 1 if second is greater
*/
int comparetopic_subscriptionsStruct(void *item1, void *item2) {
    topic_subscriptions x = *((topic_subscriptions *) item1);
    topic_subscriptions y = *((topic_subscriptions *) item2);
    return strcmp(x.id, y.id);
}

/*!
 *  \brief generate int pointer from int value
 *  \param[in]      int value
 *  \return         int pointer
*/
int *new_int(int value) {
    int *x = malloc(sizeof (int));
    *x = value;
    return x;
}


/*!
 *  \brief Compare two int values
 *  \param[in]      First int to compare
 *  \param[in]      Second int for compare
 *  \return         0 if the ints are the same, 1 if first is greater, -1 if second is greater
*/
int compare_int(void *item1, void *item2) {
    int x = *((int *) item1);
    int y = *((int *) item2);
    return (x > y) - (y > x);
}

/*!
 *  \brief print integer value from pointer
 *  \param[in]      Output where print (file or other)
 *  \param[in]      integer to be printed
*/
void to_string_print_int(FILE * output, void *item) {
    int x = *((int *) item);
    fprintf(output, "value: %d\n", x);
}

/*!
 *  \brief Parser for publish messages received. It divides the messages in special protected topics with a determined function and normal ones
 *  \param[in]      Pulga MQTT Broker parameter
 *  \param[in]      socket in which the publish has been received
 *  \param[in]      dup parameter of publish message for MQTT
 *  \param[in]      qos parameter for MQTT
 *  \param[in]      retained parameter for MQTT
 *  \param[in]      packetid parameter for MQTT 
 *  \param[in]      topic of the published message
 *  \param[in]      message received
 *  \param[in]      length of the message received
 *  \return         1 if the publish is parsed correctly, -1 if not
*/
int publish_parser(pulgamqttbroker *pulga, unsigned int s, unsigned char dup, int qos, unsigned char retained, unsigned short packetid, MQTTString topicName, unsigned char* payload, int payloadlen) {

    topic_subscriptions *aux_t = malloc(sizeof (topic_subscriptions));
    int pos;
    char str_copy[50];
    unsigned char SendBuf[BUF_SIZE];
    int len = 0;
    int result=1;
    int retval=0;
    
    strncpy(str_copy, topicName.lenstring.data, topicName.lenstring.len);
    str_copy[topicName.lenstring.len] = '\0';
    aux_t = create_topic_subscription(str_copy);

    //COMPARE EACH ELEMENT WITH THE PARSER
    int EOTcommand=1;
    
    if (strcmp(str_copy, "EOTUploadFileSD") == 0) 
    {
        //aux parameters
        int sizeInBytes = 0;
        bool boolean = false;
        int i;
        int size_payload=0;
        
        //parse message: NumberOfPackages PathToFile 
        int variables;
        int max_args=2;
        char** arguments = parse_args(&variables, max_args, payload, payloadlen);
        
        //if correct number of args
        if (variables == max_args) {

            //save variables
            char* NumberOfPackages = arguments[0];
            char* PathToFile = arguments[1];

            //number of packets to receive
            int number_of_packets = atoi(NumberOfPackages);

            //aux variables
            unsigned char dup2;
            int qos2;
            unsigned char retained2;
            unsigned short packetid2;
            MQTTString topicName2;

            //receive file
            int payload_max = 1024;
            int chars_number_of_packet = 6;
            printf("Payload length: %d \n", number_of_packets);
            int payload_file_size = 0;
            unsigned char** payload_file = receive_file(pulga, aux_t, number_of_packets, payload_max, chars_number_of_packet, s, &payload_file_size, &dup2, &qos2, &retained2, &packetid2, &topicName2); //each position saves a pointer to 1KB, Memmory=number_of_packets*1024


            if (payload_file == NULL) {
                boolean = 0;
            } else {


                //SDCardMount();
                ////printf("SDCard mounted %d \n",SDCardIsMounted());

                //printf("\nCreating file %s\n", PathToFile);
                SDCardFile* fileHandler = SDCardFileOpen(PathToFile, "w", false);
                //assert( fileHandler );	

                size_payload = payload_max;
                for (i = 0; i < number_of_packets; i++) {
                    if (payload_file_size < payload_max)
                        size_payload = payload_file_size;


                    sizeInBytes = SDCardFileWrite(fileHandler, payload_file[i], size_payload);
                    payload_file_size -= payload_max;
                    free(payload_file[i]);
                    //assert(sizeInBytes);
                }
                free(payload_file);

                //printf("\nClosing file\n");
                boolean = SDCardFileClose(&fileHandler);
                //assert( (boolean == true) );


            }

            //if file is wroten, send 0 to the client
            //if file is wroten, send 0 to the client
            if (boolean) {
                char payloadOk[] = "0\0";
                publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, payloadOk, strlen(payloadOk));
            } else {
                char payloadOk[] = "-1\0";
                publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, payloadOk, strlen(payloadOk));
            }

            //TODO: change creation and modification time of the created file

            /*
            SDCardMount();
        
            //TRY 1
             * 
             *  UART: Error utime: 134 
                UART: error: Not supported
                UART: /mnt/sdcard/111.jpg: Not supported
             * 
             * Error found in sys/errno.h of Myriad
             *  #define ENOTSUP 134		// Not supported
             * 
             *  #define ENOTSUP         252     // Function not implemented (POSIX.4 / HPUX)

             * 
            struct stat foo;
            time_t mtime;
            struct utimbuf new_times;

            if (stat(PathToFile, &foo) < 0) {
                printf("Error stat \n");
                perror(PathToFile);
                return 1;
            }
            mtime = foo.st_mtime; // seconds since the epoch

            new_times.actime = foo.st_atime; // keep atime unchanged 
            new_times.modtime = time(NULL); // set mtime to current time 
            int error = utime(PathToFile, &new_times);
            if (error < 0) {
                printf("Error utime: %d \n",errno);
                fprintf(stderr, "error: %s\n", strerror(errno));
                perror(PathToFile);
                return 1;
            }
             */

            /*
            //ORIGINAL TRY
            struct stat foo;
            if (stat(PathToFile, &foo) < 0) {
                printf("Error stat \n");
            }
        
            struct utimbuf times;
            double aux_time=getCurrentTime_t();
            times.actime=aux_time;
            times.modtime=aux_time;
            printf("aux_time: %.f \n",aux_time);
            printf("times.actime: %.f \n",times.actime);
            printf("utime: %d \n",utime(PathToFile, &times));
             */

        }
        
    }
    else if (strcmp(str_copy, "EOTMakeDirSD") == 0) 
    {
        
        //parse message: PathToFile 
        int variables;
        int max_args=1;
        char** arguments = parse_args(&variables, max_args, payload, payloadlen);
        
        //if correct number of args
        if (variables == max_args) {

            //save variables
            char* PathToDir = arguments[0];

            bool boolValue = SDCardDirMakeDirectory(PathToDir);

            //if file is wroten, send 0 to the client
            if (boolValue) {
                char payloadOk[] = "0\0";
                publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, payloadOk, strlen(payloadOk));
            } else {
                char payloadOk[] = "-1\0";
                publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, payloadOk, strlen(payloadOk));
            }

        }
        
    }
    else if (strcmp(str_copy, "EOTListFilesSD") == 0) 
    {
        //aux parameters
        int i;
        
        //parse message: PathToFile 
        int variables;
        int max_args=1;
        char** arguments = parse_args(&variables, max_args, payload, payloadlen);
        
        //if correct number of args
        if (variables == max_args) {

            //save variables
            char* PathToDir = arguments[0];

            //obtain list of directories
            int size=0;

            SDCardEntryStatus* SDlist = SDCardLs(PathToDir, &size);

            //send number of files to receive
            int payloadlenNumFiles = count_length_number(size);
            unsigned char* payloadNumFiles = malloc(sizeof (unsigned char)*(payloadlenNumFiles + 1));
            sprintf(payloadNumFiles, "%d", size);

            publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, payloadNumFiles, payloadlenNumFiles);


            //send each file information to the client
            unsigned char fileToSend[BUF_SIZE];
            for (i = 0; i < (size); i++) {
                struct tm cTime = SDlist[i].changeTime;
                struct tm aTime = SDlist[i].accessTime;
                struct tm mTime = SDlist[i].modificationTime;

                sprintf(fileToSend, "%s;%d;%s%s%s", SDlist[i].name, SDlist[i].isFile, asctime(&cTime), asctime(&aTime), asctime(&mTime));

                publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, fileToSend, strlen(fileToSend));
		//REAL SLEEP (needed to avoid packet loss)
		usleep(10000);
		//END REAL SLEEP
            }

        }
        
        
    }
    else if (strcmp(str_copy, "EOTDownloadFileSD") == 0) 
    {
        //aux parameters
        int sizeInBytes = 0;
        bool boolean = false;
        int i;
        int size_payload=0;
        int payload_max = 1024;
        
        //parse message: PathToFile 
        int variables;
        int max_args=1;
        char** arguments = parse_args(&variables, max_args, payload, payloadlen);
        
        //if correct number of args
        if (variables == max_args) {

            //save variables
            char* PathToFile = arguments[0];


            //opening for obtaining the filesize
            printf("Opening file 1 %s\n", PathToFile);
            SDCardFile* fileHandler = SDCardFileOpen(PathToFile, "r", false);
            //assert( fileHandler );	

            int payload_file_size = SDCardFileGetSize(fileHandler);
            int payload_file_size_aux = payload_file_size;
            //number of packets to send
            int number_of_packets = payload_file_size / payload_max + 1;

            //file variable
            unsigned char** payload_file = (unsigned char**) calloc(number_of_packets, sizeof (char*));

            size_payload = payload_max;
            for (i = 0; i < number_of_packets; i++) {
                payload_file[i] = NULL;
                while (payload_file[i] == NULL) {
                    //payload_image[received_packet] = (unsigned char *) malloc(payload_max);
                    payload_file[i] = malloc(sizeof (unsigned char)*payload_max);
                    //printf("Allocating memmory %d \n",actual_packet);
                }
                if (payload_file_size_aux < payload_max)
                    size_payload = payload_file_size_aux;

                if (i % 100 == 0)
                    printf("\nReading %d bytes to file (package %d)\n", size_payload, i + 1);

                sizeInBytes = SDCardFileRead(fileHandler, payload_file[i], size_payload);
                payload_file_size_aux -= payload_max;

                //printf("sizeInBytes %d \n", sizeInBytes);
                //assert(sizeInBytes);
            }

            printf("\nClosing file\n");
            boolean = SDCardFileClose(&fileHandler);
            //assert( (boolean == true) );

            //send all packages from the complete payload_image buffer 
            int sleep_packages = 0;
            int file_sent = send_file(pulga, aux_t, s, sleep_packages, payload_file, number_of_packets, payload_max, payload_file_size, dup, qos, retained, packetid, topicName);

            if (file_sent) {
                printf("File sent \n");
            }

        }
        
    }
    else if (strcmp(str_copy, "EOTDeleteFileSD") == 0) 
    {
        //parse message: PathToFile 
        int variables;
        int max_args=1;
        char** arguments = parse_args(&variables, max_args, payload, payloadlen);
        
        //if correct number of args
        if (variables == max_args) {

            //save variables
            char* PathToFile = arguments[0];

            bool boolValue = SDCardFileRemove(PathToFile);

            //if file is wroten, send 0 to the client
            if (boolValue) {
                char payloadOk[] = "0\0";
                publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, payloadOk, strlen(payloadOk));
            } else {
                char payloadOk[] = "-1\0";
                publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, payloadOk, strlen(payloadOk));
            }

        }
    }
    else if (strcmp(str_copy, "EOTDeleteDirSD") == 0) 
    {
        //parse message: PathToFile 
        int variables;
        int max_args=1;
        char** arguments = parse_args(&variables, max_args, payload, payloadlen);
        
        //if correct number of args
        if (variables == max_args) {

            //save variables
            char* PathToDir = arguments[0];

            bool boolValue = SDCardDirRemoveDirectoryRecursive(PathToDir);

            //if file is wroten, send 0 to the client
            if (boolValue) {
                char payloadOk[] = "0\0";
                publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, payloadOk, strlen(payloadOk));
            } else {
                char payloadOk[] = "-1\0";
                publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, payloadOk, strlen(payloadOk));
            }

        }
    }
    else if (strcmp(str_copy, "EOTDeleteContentSD") == 0) 
    {
        //parse message: PathToFile 
        int variables;
        int max_args=1;
        char** arguments = parse_args(&variables, max_args, payload, payloadlen);
        
        //if correct number of args
        if (variables == max_args) {

            //save variables
            char* PathToDir = arguments[0];

            bool boolValue = SDCardRemoveAllFromDirectory(PathToDir);

            //if file is wroten, send 0 to the client
            if (boolValue) {
                char payloadOk[] = "0\0";
                publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, payloadOk, strlen(payloadOk));
            } else {
                char payloadOk[] = "-1\0";
                publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, payloadOk, strlen(payloadOk));
            }

        }
    }
    else if (strcmp(str_copy, "EOTUploadElf") == 0) 
    {
        //TODO, only for testing the Java APP
        
        
        //aux parameters
        int sizeInBytes = 0;
        bool boolean = false;
        int i;
        int size_payload=0;
        
        //parse message: NumberOfPackages PathToFile 
        int variables;
        int max_args=2;
        char** arguments = parse_args(&variables, max_args, payload, payloadlen);
        
        //if correct number of args
        if (variables == max_args) {

            //save variables
            char* NumberOfPackages = arguments[0];
            char* PathToFile = arguments[1];

            //number of packets to receive
            int number_of_packets = atoi(NumberOfPackages);

            //aux variables
            unsigned char dup2;
            int qos2;
            unsigned char retained2;
            unsigned short packetid2;
            MQTTString topicName2;

            //receive file
            int payload_max = 1024;
            int chars_number_of_packet = 6;
            printf("Payload length: %d \n", number_of_packets);
            int payload_file_size = 0;
            unsigned char** payload_file = receive_file(pulga, aux_t, number_of_packets, payload_max, chars_number_of_packet, s, &payload_file_size, &dup2, &qos2, &retained2, &packetid2, &topicName2); //each position saves a pointer to 1KB, Memmory=number_of_packets*1024


            
            
            if (payload_file == NULL) {
                boolean = 0;
            } else {

                //create path to elf file
                const char* folder = "/mnt/sdcard/testFlash/";
                //create dir for testing flash
                bool boolValue = SDCardDirMakeDirectory("/mnt/sdcard/testFlash");


                char* PathToFile2 = malloc(strlen(folder) + 1 + strlen(PathToFile)); /* make space for the new string (should check the return value ...) */
                strcpy(PathToFile2, folder); /* copy name into the new var */
                strcat(PathToFile2, PathToFile); /* add the extension */



                //printf("\nCreating file %s\n", PathToFile2);
                SDCardFile* fileHandler = SDCardFileOpen(PathToFile2, "w", false);
                //assert( fileHandler );	

                size_payload = payload_max;
                for (i = 0; i < number_of_packets; i++) {
                    if (payload_file_size < payload_max)
                        size_payload = payload_file_size;



                    sizeInBytes = SDCardFileWrite(fileHandler, payload_file[i], size_payload);
                    payload_file_size -= payload_max;
                    free(payload_file[i]);
                    //assert(sizeInBytes);
                }
                
                free(payload_file);

                //printf("\nClosing file\n");
                boolean = SDCardFileClose(&fileHandler);

               
            }
            

            //if file is wroten, send 0 to the client
            if (boolean) {
                char payloadOk[] = "0\0";
                publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, payloadOk, strlen(payloadOk));
            } else {
                char payloadOk[] = "-1\0";
                publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, payloadOk, strlen(payloadOk));
            }

        }
        
    }
    else if (strcmp(str_copy, "EOTListElf") == 0) 
    {
        //TODO
    }
    else if (strcmp(str_copy, "EOTSetStartElf") == 0) 
    {
        //TODO
    }
    else if (strcmp(str_copy, "EOTSnapshot") == 0) 
    {
            int sleep_packages = 0;
            int payload_max = 1024;
            int file_sent = send_snapshot(pulga, aux_t, s, sleep_packages, payload_max, dup, qos, retained, packetid, topicName);
            if(file_sent){
                printf("Snapshot sent \n");
            }
    }
    else if (strcmp(str_copy, "EOTShowImage") == 0) 
    {
       MQTTString topicshowaux = MQTTString_initializer;        
       topicShowImage = topicshowaux;

        printf("%s \n",topicName.lenstring.data);
        printf("%d \n",topicName.lenstring.len);

        topicShowImage.lenstring.data = (char*)malloc((strlen(topicName.lenstring.data)+1)*sizeof(char));

        printf("Topic Saved \n");
        strcpy(topicShowImage.lenstring.data, topicName.lenstring.data);
        topicShowImage.lenstring.len = topicName.lenstring.len;


        printf("%s \n",topicShowImage.lenstring.data);
        printf("%d \n",topicShowImage.lenstring.len);

        /*unsigned char* pptr;
        unsigned char* pptrend;
        writeMQTTString(&pptr, topicName);
        pptrend = pptr+strlen(pptr);
        readMQTTLenString(&topicShowImage, &pptr, pptrend);
        printf("Topic created \n");*/


        showImagepacketid = packetid;
        printf("Topic Saved \n");
        EOTcommand=0;
        /*if (images_saved >= 0) {
            int i;
            int sleep_packages = 1;
            int payload_max = 1024;
            for (i = 0; i <= images_saved; i++) {
                int file_sent = send_show_image(i, pulga, aux_t, s, sleep_packages, payload_max, dup, qos, retained, packetid, topicName);
                if (file_sent) {
                    printf("Snapshot sent \n");
                }
                free(image_to_show_buffer[i]);
            }
            images_saved = -1;
        }*/
        /*
        
            int sleep_packages = 1;
            int payload_max = 1024;
            int file_sent = send_show_image(pulga, aux_t, s, sleep_packages, payload_max, dup, qos, retained, packetid, topicName);
            if(file_sent){
                printf("Snapshot sent \n");
            }
         */
    }
    else if (strcmp(str_copy, "EOTCreateAP") == 0) 
    {
        //TODO
        
        //parse message: Ssid Security [Pass] [Channel]        
        int variables;
        int max_args=4;
        char** arguments = parse_args(&variables, max_args, payload, payloadlen);
        
        //save variables
        char* ssid=arguments[0];
        char* security=arguments[1];
        char* pass=arguments[2];
        char* channel=arguments[3];

        if(variables==max_args)
        {
            //retval=generateAP(ssid, pass, atoi(security), atoi(channel));
            retval=generateAPSaveProfile(ssid, pass, atoi(security), atoi(channel));
            
            //if created correctly, restart Pulga
            if ( retval >= 0)
                result = -1;
        }
            
        
        if (variables >= 1)
            printf("SSID: %s \n", ssid);
        if (variables >= 2)
            printf("Security: %s \n", security);
        if (variables >= 3)
            printf("Pass: %s \n", pass);
        if (variables >= 4)
            printf("Channel: %s \n", channel);
        
        
        
    }
    else if (strcmp(str_copy, "EOTConnectToAP") == 0) 
    {
        //TODO
        
        //parse message: Ssid Security [Pass]
        int variables;
        int max_args=3;
        char** arguments = parse_args(&variables, max_args, payload, payloadlen);
        
        //save variables
        char* ssid=arguments[0];
        char* security=arguments[1];
        char* pass=arguments[2];

        if(variables==max_args)
        {
            retval=connectToAP(ssid, pass, atoi(security), 30);
            //if connected correctly, restart Pulga
            if ( retval >= 0)
            {
                result = -1;
            }
            else
            {
                generateAPFromProfileOnErrorDefault(0);
                result = -1;
            }
        }
            
        
        if (variables >= 1)
            printf("SSID: %s \n", ssid);
        if (variables >= 2)
            printf("Security: %s \n", security);
        if (variables >= 3)
            printf("Pass: %s \n", pass);
        
    }
    else if (strcmp(str_copy, "EOTDisconnectFromAP") == 0) 
    {
        //TODO
        //remove profile and connect to Default
        retval=removeProfiles();
        generateAPFromProfileOnErrorDefault(0);
        //restart pulga
        result = -1;
    }
    else if (strcmp(str_copy, "EOTUpdateDate") == 0) 
    {
        //TODO 
        
        //parse message: Year Month Day Hour Min Sec
        int variables;
        int max_args=6;
        char** arguments = parse_args(&variables, max_args, payload, payloadlen);
        
        //save variables
        char* year=arguments[0];
        char* month=arguments[1];
        char* day=arguments[2];
        char* hour=arguments[3];
        char* min=arguments[4];
        char* sec=arguments[5];

        if (variables == max_args) {
            setCurrentTime(atoi(hour), atoi(min), atoi(sec), atoi(year), atoi(month), atoi(day));

            //send 0 to the client, date updated
            char payloadOk[] = "0\0";
            publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, payloadOk, strlen(payloadOk));

        } else {
            char payloadOk[] = "-1\0";
            publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, payloadOk, strlen(payloadOk));
        }
        
    }
    
    
    else if (strcmp(str_copy, "EOTGetDate") == 0) 
    {
        //TODO 
        
        //if file is wroten, send 0 to the client
        char* dateChar=getCurrentTime_char();
        publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, dateChar, strlen(dateChar));
        
        
    }
    
    
    else if (strcmp(str_copy, "EOTPython") == 0) 
    {
        
        EOTcommand=0;

        //aux parameters
        int sizeInBytes = 0;
        bool boolean = true;
        int i;
        int size_payload=0;
        
        //parse message: NumberOfPackages PathToFile 
        int variables;
        int max_args=2;
        char** arguments = parse_args(&variables, max_args, payload, payloadlen);
        
        //if correct number of args
        if (variables == max_args) {

            //save variables
            char* NumberOfPackages = arguments[0];
            int disableOutputs = atoi(arguments[1]);

            //number of packets to receive
            int number_of_packets = atoi(NumberOfPackages);

            //aux variables
            unsigned char dup2;
            int qos2;
            unsigned char retained2;
            unsigned short packetid2;
            MQTTString topicName2;

            //receive file
            int payload_max = 1024;
            int chars_number_of_packet = 6;
            printf("Payload length: %d \n", number_of_packets);
            int payload_file_size = 0;
            unsigned char** payload_file = receive_file(pulga, aux_t, number_of_packets, payload_max, chars_number_of_packet, s, &payload_file_size, &dup2, &qos2, &retained2, &packetid2, &topicName2); //each position saves a pointer to 1KB, Memmory=number_of_packets*1024

            if (payload_file == NULL) {
                boolean = false;
            }
            if (boolean) {
                char payloadOk[] = "0\0";
                publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, payloadOk, strlen(payloadOk));
            } else {
                char payloadOk[] = "-1\0";
                publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, payloadOk, strlen(payloadOk));
            }


            unsigned char* payload_code = (unsigned char*) malloc((payload_file_size + 1) * sizeof (char));
            memset(payload_code, 0, payload_file_size + 1);
            int k;
            for (k = 0; k < number_of_packets; k++) {
                strncat(payload_code, payload_file[k], payload_max);
            }
            payload_code[payload_file_size] = 0;
            printf("%s\n", payload_code);


            //execute python code


            pthread_attr_t attr;
            int rc1;
            pthread_t thread1;

            if (pthread_attr_init(&attr) != 0) {
                printk("pthread_attr_init error");
            }
            if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) != 0) {
                printk("pthread_attr_setinheritsched error");
            }
            if (pthread_attr_setschedpolicy(&attr, SCHED_RR) != 0) {
                printk("pthread_attr_setschedpolicy error");
            }


            if ((rc1 = pthread_create(&thread1, &attr, &execPython, payload_code))) {
                printk("Thread PythonLexer creation failed: %d\n", rc1);
            } else {
                printk("Thread PythonLexer created\n");
            }


            for (;;) {
                // Locks Pulga until a Python signal
                pthread_mutex_lock(&mutex);
                while (state != SHOW_IMAGE && state != PYTHON_END && state != SEND_OUTPUT)
                    pthread_cond_wait(&pulgaTurn, &mutex);
                pthread_mutex_unlock(&mutex);

                // Checks Python states
                if (state == SHOW_IMAGE) {
                    int sleep_packages = 1;
                    int payload_max = 1024;
                    int file_sent = send_show_image(pulga, aux_t, s, sleep_packages, payload_max, dup, qos, retained, showImagepacketid, topicShowImage);
                } else if (state == SEND_OUTPUT){
                    if(!disableOutputs) 
                        publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, buff_micropython, strlen(buff_micropython));
                    memset(buff_micropython, 0x00, sizeof (buff_micropython));
                    strcpy(buff_micropython, ">");
                }
                else { // PYTHON_END
                    char endMessage[5];
                    memset(endMessage, 0x00, sizeof (endMessage));
                    strcpy(endMessage, "<\n");
                    publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, endMessage, strlen(endMessage));
                    // End of the waiting loop
                    break;
                }

                // Allows Python to continue 
                pthread_mutex_lock(&mutex);
                state = EXECUTE_PYTHON;
                pthread_cond_signal(&pythonTurn);
                pthread_mutex_unlock(&mutex);
            }

            // Restart the state
            state = EXECUTE_PYTHON;


            for (i = 0; i < number_of_packets; i++) {
                free(payload_file[i]);
            }
            free(payload_file);


            // wait for Python thread to finish 
            rc1 = pthread_join(thread1, NULL);
            if (rc1 != 0) {
                printk("pthread_join error (%d)!\n", rc1);
            }

            
            EOTcommand = 0;
        }

    }
        // NORMAL PUBLISH MESSAGE
    else {
        publishToSubscribers(pulga, aux_t, dup, qos, retained, packetid, topicName, payload, payloadlen);
        EOTcommand = 0;
    }

    //clear all subscribers if it is an EOT command
    if (EOTcommand) {

        int pos = ll_item_position(pulga->topics_subscribers, aux_t, comparetopic_subscriptionsStruct);
        if (pos >= 0) { //remove all subscribers from topic           
            topic_subscriptions* aux_topic = (topic_subscriptions *) ll_get_index(pulga->topics_subscribers, pos);
            //printf("EOT command, removing all subscribers, size of list at start %d  \n",aux_topic->suscribed->size);
            ll_pop_all(aux_topic->suscribed);
            //printf("Size of list at end %d  \n",aux_topic->suscribed->size);
        }
    }
    
    return result;
}



/*!
 *  \brief Parse a number of arguments separated by spaces from MQTT message
 *  \param[out]     number of variables parsed
 *  \param[in]      max number of arguments to parse
 *  \param[in]      message to be parsed
 *  \param[in]      length of the message to be parsed
 *  \return         Arguments parsed
*/
char** parse_args(int* variables, int max_args, unsigned char* payload, int payloadlen)
{
    char** arguments = (char**) calloc(max_args, sizeof (char*));
    unsigned char* payload_aux = payload;
    int i = 0;
    *variables = 0;
    int previous_space = 1;


    //first arguments separated by " "
    for (i = 0; i < payloadlen && *variables < max_args; i++) {

        if (payload[i] == ' ') {
            if (previous_space == 0) {
                //copy the arg
                arguments[*variables] = malloc((sizeof (char))*(i + 1 - (payload_aux - payload)));
                memcpy(arguments[*variables], payload_aux, i - (payload_aux - payload));
                arguments[*variables][i - (payload_aux - payload)] = '\0';
                (*variables)++;
            }

            //increment pointer
            payload_aux += (i + 1 - (payload_aux - payload));
            previous_space = 1;

        } else {
            previous_space = 0;
        }
    }

    //last argument if last element of payload was different from " "
    //if(strcmp(payload[i-1]," ")!=0 && *variables < max_args)
    if (payload[i - 1] != ' ' && *variables < max_args) {
        arguments[*variables] = malloc((sizeof (char))*(i + 1 - (payload_aux - payload)));
        memcpy(arguments[*variables], payload_aux, i - (payload_aux - payload));
        arguments[*variables][i - (payload_aux - payload)] = '\0';

        (*variables)++;
    }
    
    return arguments;
}

/*!
 *  \brief Publish a message in a certain topic to its subscribers
 *  \param[in]      Pulga MQTT Broker parameter
 *  \param[in]      Topic subscribed
 *  \param[in]      dup parameter of publish message for MQTT
 *  \param[in]      qos parameter for MQTT
 *  \param[in]      retained parameter for MQTT
 *  \param[in]      packetid parameter for MQTT 
 *  \param[in]      topic of the published message
 *  \param[in]      message received
 *  \param[in]      length of the message received
 *  \return         1 if the publish is sent correctly
*/
int publishToSubscribers(pulgamqttbroker *pulga, topic_subscriptions *aux_t, unsigned char dup, int qos, unsigned char retained, unsigned short packetid, MQTTString topicName, unsigned char* payload, int payloadlen){
    unsigned char SendBuf[BUF_SIZE];
    int len=0;
    int pos;
    
    memset(SendBuf, 0x00, sizeof (SendBuf));
    len = MQTTSerialize_publish(SendBuf, sizeof (SendBuf), dup, qos, retained, packetid, topicName, payload, payloadlen);

    pos = ll_item_position(pulga->topics_subscribers, aux_t, comparetopic_subscriptionsStruct);

    if (pos >= 0) { //if the topic exists
        unsigned int i;
        topic_subscriptions* aux_topic = (topic_subscriptions *) ll_get_index(pulga->topics_subscribers, pos);
        
        //printf("Sending publish message to subscribers: \n");
        FILE *output = stdout;
        //ll_print(aux_topic->suscribed, output, to_string_print_int);

        for (i = 0; i < aux_topic->suscribed->size; i++) {
            //int test=*((unsigned int *)ll_get_index(aux_topic->suscribed,i));
            //Sleep(sleep_milsec);
            sl_Send(*((unsigned int *) ll_get_index(aux_topic->suscribed, i)), SendBuf, len, 0);
        }
    }
    return 1;
}


/*!
 *  \brief obtain number of received packets
 *  \param[in]      message received
 *  \param[in]      length of the message received
 *  \param[in]      number of digits at the end of payload which represents the number of packets
 *  \return         digits parsed
*/
int calculate_rec_packet(unsigned char* payload,int payloadlen,int chars_number_of_packet, MQTTString* topicName2){
    int result=0;
    int i;
    unsigned char* payloadaux=payload;
	
    payloadaux+=payloadlen-chars_number_of_packet;
    //for (i=payloadlen-chars_number_of_packet;i<payloadlen;i++){
        result+=atoi((const char*) payloadaux);//*pow(10,payloadlen-i-1);


    return result;
}
/*!
 *  \brief receive file after indicated in a message
 *  \param[in]      number of packets to receive
 *  \param[in]      max size of the payload of each part of the file (1024 bytes for example)
 *  \param[in]      number of chars used at the end of the payload for indicating the number of the packet (6 for example)
 *  \param[in]      socket used for communication
 *  \param[out]     complete size of the file/picture received
 *  \param[in]      dup parameter of publish message for MQTT
 *  \param[in]      qos parameter for MQTT
 *  \param[in]      retained parameter for MQTT
 *  \param[in]      packetid parameter for MQTT 
 *  \param[in]      topic of the published message
 *  \return         file received divided in packages
 */

unsigned char** receive_file(pulgamqttbroker *pulga, topic_subscriptions *aux_t, int number_of_packets, int payload_max, int chars_number_of_packet, unsigned int s, int* payload_image_size, unsigned char* dup2, int* qos2, unsigned char* retained2, unsigned short* packetid2, MQTTString* topicName2){
    //variable to return    
    unsigned char** payload_image=(unsigned char**) calloc(number_of_packets, sizeof(char*));
    int primero = 1;
    //auxiliar variables
    int actual_packet = 0;
    unsigned char* payload2;
    unsigned char RecvBuf_file[BUF_SIZE];
    int received_packet = -1;
    int previous_received_packet=-1;
    int payloadlen2 = 1;
    int frc;
    int rc;
    char payloadOk[] = "2\0";

    *payload_image_size = 0;

    //allocate memory
    while (actual_packet < number_of_packets) {
        payload_image[actual_packet] = NULL;
        while (payload_image[actual_packet] == NULL) {
            //payload_image[received_packet] = (unsigned char *) malloc(payload_max);
            payload_image[actual_packet] = malloc(sizeof (unsigned char)*payload_max);
            memset(payload_image[actual_packet], 0, payload_max);
            //printf("Allocating memmory %d \n",actual_packet);
        }
        actual_packet++;
    }

    actual_packet = 0;
    //receive all packages
    long nonBlocking = 0;
    sl_SetSockOpt(s, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &nonBlocking, sizeof (nonBlocking));
    while (actual_packet < number_of_packets && (received_packet + 1) < number_of_packets && payload_image!=NULL) {
        //Sleep(3000);
        frc = -1;
        if (primero==1)
            received_packet=-1;
        while (frc != PUBLISH && payload_image != NULL){// || (received_packet==0 && primero==0)) {
            
            //DrvTimerSleepMs(100);
            memset(RecvBuf_file, 0x00, sizeof (RecvBuf_file));
            frc = MQTTPacket_read2(RecvBuf_file, sizeof (RecvBuf_file), transport_getdata, s);

            rc = MQTTDeserialize_publish(dup2, qos2, retained2, packetid2, topicName2, &payload2, &payloadlen2, RecvBuf_file, sizeof (RecvBuf_file));
            //printf("Message received on topic %s: \n Payload: %s \n Length of total payload: %d \n Length of topic: %d \n",topicName2.lenstring.data,payload2,payloadlen2,topicName2.lenstring.len);
            received_packet = calculate_rec_packet(payload2, payloadlen2, chars_number_of_packet, topicName2);
            
            //print error
            if(received_packet==0 && primero==0){
                int j;
                for(j=0;j<=previous_received_packet;j++){
                    free(payload_image[j]);
                }
                free(payload_image);
                payload_image=NULL;
                //printf("Error detected: Current packet received: %d Current frc: %d Current actual packet: %d Number of packets: %d \n", received_packet, frc, actual_packet,number_of_packets);
            }
            previous_received_packet=received_packet;
            
        }
        
        if (payload_image != NULL) {
            primero = 0;
            
            //if (received_packet % 1000 == 0)
            //    printf("Current packet received: %d Current frc: %d Current actual packet: %d %d\n", received_packet, frc, actual_packet, payloadlen2);

            memcpy(payload_image[received_packet], payload2, payloadlen2 - chars_number_of_packet);
            //payload_image_buf+=payloadlen2;
            *payload_image_size += payloadlen2 - chars_number_of_packet;
            actual_packet++;
            
            //answer to client for sending another packet
            publishToSubscribers(pulga, aux_t, dup2, qos2, retained2, packetid2, *topicName2, payloadOk, strlen(payloadOk));
        }
    }
    nonBlocking = 1;
    sl_SetSockOpt(s, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &nonBlocking, sizeof (nonBlocking));

    return payload_image;
}


/*!
 *  \brief send file indicating the number of packets when sending
 *  \param[in]      Pulga MQTT Broker parameter
 *  \param[in]      subscribers subscribed to the topic
 *  \param[in]      socket used for communication
 *  \param[in]      0 if not socket sleep implemented, 1 if it is implemented
 *  \param[in]      file to be sent  
 *  \param[in]      number of packets to be sent
 *  \param[in]      max size of the payload of each part of the file (1024 bytes for example)
 *  \param[in]      complete size of the file to be sent
 *  \param[in]      dup parameter of publish message for MQTT
 *  \param[in]      qos parameter for MQTT
 *  \param[in]      retained parameter for MQTT
 *  \param[in]      packetid parameter for MQTT 
 *  \param[in]      topic of the published message
 *  \return         1 if the file is sent correctly
*/
int send_file(pulgamqttbroker *pulga, topic_subscriptions *aux_t, unsigned int s, int sleep_packages, unsigned char** payload_image, int number_of_packets, int payload_max, int payload_image_size, unsigned char dup2, int qos2, unsigned char retained2, unsigned short packetid2, MQTTString topicName2){
    //variables
    unsigned char SendBuf_file[BUF_SIZE];
    unsigned char RecvBuf_file[BUF_SIZE];
    int len = 0;
    int frc;
    int pos;
    int payload_image_size_send = 0;
    int actual_packet = 0;
    
    //payload and number of chars of the payload that indicate the number of packets
    int payloadlen=count_length_number(number_of_packets);
    unsigned char* payload=malloc(sizeof (unsigned char)*(payloadlen+1));
    sprintf(payload,"%d",number_of_packets);

    // send information package (number of packages)

    memset(SendBuf_file, 0x00, sizeof (SendBuf_file));
    len = MQTTSerialize_publish(SendBuf_file, sizeof (SendBuf_file), dup2, qos2, retained2, packetid2, topicName2, payload, payloadlen);

    pos = ll_item_position(pulga->topics_subscribers, aux_t, comparetopic_subscriptionsStruct);

    if (pos >= 0) { //if the topic exists
        unsigned int i;
        topic_subscriptions* aux_topic = (topic_subscriptions *) ll_get_index(pulga->topics_subscribers, pos);

        for (i = 0; i < aux_topic->suscribed->size; i++) {
            //int test=*((unsigned int *)ll_get_index(aux_topic->suscribed,i));
            //Sleep(sleep_milsec);
            sl_Send(*((unsigned int *) ll_get_index(aux_topic->suscribed, i)), SendBuf_file, len, 0);
        }
    }

    // send the rest of packages
    
    payload_image_size_send = payload_max;
    long nonBlocking = 0;
    while (actual_packet < number_of_packets) {
        //printf("Current packet sent: %d \n", actual_packet);
        if ((actual_packet + 1) == number_of_packets) {
            payload_image_size_send = payload_image_size - actual_packet*payload_max;
        }

        //FICTITIOUS SLEEP
        if (sleep_packages) {
            nonBlocking = 0;
            sl_SetSockOpt(s, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &nonBlocking, sizeof (nonBlocking));
            frc = -1;
            while (frc != PUBLISH) {
                //DrvTimerSleepMs(100);
                memset(RecvBuf_file, 0x00, sizeof (RecvBuf_file));
                frc = MQTTPacket_read2(RecvBuf_file, sizeof (RecvBuf_file), transport_getdata, s);
            }
            nonBlocking = 1;
            sl_SetSockOpt(s, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &nonBlocking, sizeof (nonBlocking));
        }
        //END FICTITIOUS SLEEP
        
	//REAL SLEEP (needed to avoid packet loss)
        usleep(10000);
        //END REAL SLEEP

        //PUBLISH TO SUSCRIBERS (dividing it)
        memset(SendBuf_file, 0x00, sizeof (SendBuf_file));
        //len = MQTTSerialize_publish(SendBuf_file,sizeof(SendBuf_file),dup2,qos2,retained2,packetid2,topicName2,payload2,payloadlen2);
        len = MQTTSerialize_publish(SendBuf_file, sizeof (SendBuf_file), dup2, qos2, retained2, packetid2, topicName2, payload_image[actual_packet], payload_image_size_send);
        //strncpy(str_copy,topicName.lenstring.data,topicName.lenstring.len);
        //str_copy[topicName.lenstring.len] = '\0';
        //aux_t = create_topic_subscription(str_copy);

        pos = ll_item_position(pulga->topics_subscribers, aux_t, comparetopic_subscriptionsStruct);

        if (pos >= 0) { //if the topic exists
            unsigned int i;
            topic_subscriptions* aux_topic = (topic_subscriptions *) ll_get_index(pulga->topics_subscribers, pos);

            for (i = 0; i < aux_topic->suscribed->size; i++) {
                //SLEEPS MYRIAD
                //sleep(1);
                //DrvTimerSleepMs(1000);
                //usleep(20000);
                //rtems_task_wake_after(10000);


                sl_Send(*((unsigned int *) ll_get_index(aux_topic->suscribed, i)), SendBuf_file, len, 0);
            }
        }
        free(payload_image[actual_packet]);
        actual_packet++;

        //payload_send_buf+=payload_max;
    }
    
    return 1;
}



int send_snapshot(pulgamqttbroker *pulga, topic_subscriptions *aux_t, unsigned int s, int sleep_packages, int payload_max, unsigned char dup2, int qos2, unsigned char retained2, unsigned short packetid2, MQTTString topicName2){
    //variables
    unsigned char SendBuf_file[BUF_SIZE];
    unsigned char RecvBuf_file[BUF_SIZE];
    int len = 0;
    int frc;
    int pos;
    int payload_image_size_send = 0;
    int actual_packet = 0;
    int number_of_packets;
    int i;

    //getJpegFrame();

    pthread_attr_t attr;
    int rc1;
    pthread_t thread1;

    if(pthread_attr_init(&attr) !=0) {
        printk("pthread_attr_init error");
    }
    if(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) != 0) {
        printk("pthread_attr_setinheritsched error");
    }
    if(pthread_attr_setschedpolicy(&attr, SCHED_RR) != 0) {
        printk("pthread_attr_setschedpolicy error");
    }

    
    if ((rc1=pthread_create(&thread1, &attr, &take_snapshot, NULL))) {
        printk("Thread 1 creation failed: %d\n", rc1);
    }
    else {
        printk("Thread 1 created\n");
    }

    // wait for thread to finish
    rc1 = pthread_join( thread1, NULL);
    if(rc1 != 0) {
        printk("pthread_join error (%d)!\n", rc1);
    }

    number_of_packets = ((image_size_in_bytes % payload_max) == 0) ? 
        image_size_in_bytes / payload_max : image_size_in_bytes / payload_max + 1;

    //payload and number of chars of the payload that indicate the number of packets
    int payloadlen = count_length_number(number_of_packets);
    printf("number_of_packets: %d\n", number_of_packets);
    unsigned char* payload = malloc(sizeof (unsigned char) * (payloadlen + 1));
    sprintf(payload, "%d", number_of_packets);

    // send information package (number of packages)

    memset(SendBuf_file, 0x00, sizeof (SendBuf_file));
    len = MQTTSerialize_publish(SendBuf_file, sizeof (SendBuf_file), dup2, qos2, retained2, packetid2, topicName2, payload, payloadlen);

    pos = ll_item_position(pulga->topics_subscribers, aux_t, comparetopic_subscriptionsStruct);

    if (pos >= 0) { //if the topic exists
        int i;
        topic_subscriptions* aux_topic = (topic_subscriptions *) ll_get_index(pulga->topics_subscribers, pos);

        for (i = 0; i < aux_topic->suscribed->size; i++) {
            sl_Send(*((unsigned int *) ll_get_index(aux_topic->suscribed, i)), SendBuf_file, len, 0);
        }
    }

    
    // send the rest of packages
    long nonBlocking = 0;
    int offset = 0;

    while ((image_size_in_bytes - offset) > 0) {

        payload_image_size_send = min(payload_max, image_size_in_bytes - offset);
        char ImgBuf[payload_max];
        memcpy(&ImgBuf[0], &image[offset], payload_image_size_send);

        //PUBLISH TO SUSCRIBERS (dividing it)
        memset(SendBuf_file, 0x00, sizeof (SendBuf_file));
        len = MQTTSerialize_publish(SendBuf_file, sizeof (SendBuf_file), dup2, qos2, retained2, packetid2, topicName2, ImgBuf, payload_image_size_send);

        pos = ll_item_position(pulga->topics_subscribers, aux_t, comparetopic_subscriptionsStruct);
        if (pos >= 0) { //if the topic exists
            int i;
            topic_subscriptions* aux_topic = (topic_subscriptions *) ll_get_index(pulga->topics_subscribers, pos);
            for (i = 0; i < aux_topic->suscribed->size; i++) {
                sl_Send(*((unsigned int *) ll_get_index(aux_topic->suscribed, i)), SendBuf_file, len, 0);
            }
        }

        offset += payload_max;
        rtems_task_wake_after(100);
    }
    free(payload);
    printf ("image bytes show snap %d\n", image_size_in_bytes);

    return 1;
}



int send_show_image(pulgamqttbroker *pulga, topic_subscriptions *aux_t, unsigned int s, int sleep_packages, int payload_max, unsigned char dup2, int qos2, unsigned char retained2, unsigned short packetid2, MQTTString topicName2){
    //variables
    unsigned char SendBuf_file[BUF_SIZE];
    unsigned char RecvBuf_file[BUF_SIZE];
    int len = 0;
    int frc;
    int pos;
    int payload_image_size_send = 0;
    int actual_packet = 0;
    int number_of_packets;
    int i;
   // int image_size = 480*256;

    //getJpegFrame();

    number_of_packets = ((image_size_in_bytes_to_show % payload_max) == 0) ? 
        image_size_in_bytes_to_show / payload_max : image_size_in_bytes_to_show / payload_max + 1;

    //payload and number of chars of the payload that indicate the number of packets
    int payloadlen = count_length_number(number_of_packets);
    //printf("number_of_packets: %d\n", number_of_packets);
    unsigned char* payload = malloc(sizeof (unsigned char) * (payloadlen + 1));
    sprintf(payload, "%d", number_of_packets);

    // send information package (number of packages)

    memset(SendBuf_file, 0x00, sizeof (SendBuf_file));
    len = MQTTSerialize_publish(SendBuf_file, sizeof (SendBuf_file), dup2, qos2, retained2, packetid2, topicName2, payload, payloadlen);

    pos = ll_item_position(pulga->topics_subscribers, aux_t, comparetopic_subscriptionsStruct);

    if (pos >= 0) { //if the topic exists
        int i;
        topic_subscriptions* aux_topic = (topic_subscriptions *) ll_get_index(pulga->topics_subscribers, pos);

        for (i = 0; i < aux_topic->suscribed->size; i++) {
            sl_Send(*((unsigned int *) ll_get_index(aux_topic->suscribed, i)), SendBuf_file, len, 0);
        }
    }

    // send the rest of packages
    long nonBlocking = 0;
    int offset = 0;

    while ((image_size_in_bytes_to_show - offset) > 0) {

        payload_image_size_send = min(payload_max, image_size_in_bytes_to_show - offset);
        char ImgBuf[payload_max];
        memcpy(&ImgBuf[0], &image_to_show[offset], payload_image_size_send);

        //PUBLISH TO SUSCRIBERS (dividing it)
        memset(SendBuf_file, 0x00, sizeof (SendBuf_file));
        len = MQTTSerialize_publish(SendBuf_file, sizeof (SendBuf_file), dup2, qos2, retained2, packetid2, topicName2, ImgBuf, payload_image_size_send);

        pos = ll_item_position(pulga->topics_subscribers, aux_t, comparetopic_subscriptionsStruct);
        if (pos >= 0) { //if the topic exists
            int i;
            topic_subscriptions* aux_topic = (topic_subscriptions *) ll_get_index(pulga->topics_subscribers, pos);
            for (i = 0; i < aux_topic->suscribed->size; i++) {
                sl_Send(*((unsigned int *) ll_get_index(aux_topic->suscribed, i)), SendBuf_file, len, 0);
            }
        }

        offset += payload_max;
        rtems_task_wake_after(100);
    }
    //printf ("image bytes show %d\n", image_size_in_bytes_to_show);
       
    return 1;
}




/*!
 *  \brief count the number of digits of a number
 *  \param[in]      int
 *  \return         number of digits of the int number
*/
int count_length_number(int num){
    int contador=1;
    while(num/10>0)
    {
        num=num/10;
        contador++;
    }
    return contador;
    
}

//PYTHON FUNCTIONS

void *execPython(void *payload_code){
	do_str((unsigned char*) payload_code, MP_PARSE_FILE_INPUT);
}

/*!
 *  \brief execute python command in micropython
 *  \param[in]      command text
 *  \param[in]      input kind
*/
void do_str(const char *src, mp_parse_input_kind_t input_kind) {
    //output
    memset(buff_micropython, 0x00, sizeof(buff_micropython));
    strcpy(buff_micropython,">");
    
    mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src, strlen(src), 0);
    if (lex == NULL) {
        printf("MemoryError: lexer could not allocate memory\n");
        return;
    }

    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        //printf("1111111111111 New instruction 1111111111111\n");
        qstr source_name = lex->source_name;
        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, MP_EMIT_OPT_NONE, true);
        
        mp_call_function_0(module_fun); //RUN PYTHON CODE
        
        nlr_pop();
    } else {
        // uncaught exception
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
    }
    
    //output
    if(strlen(buff_micropython)>1)
        printf("%s \n",buff_micropython);

    /* Set state to C and wake up thread B */
    pthread_mutex_lock(&mutex);
    state = PYTHON_END;
    pthread_cond_signal(&pulgaTurn);
    pthread_mutex_unlock(&mutex);
}

void gc_collect(void) {
    // WARNING: This gc_collect implementation doesn't try to get root
    // pointers from CPU registers, and thus may function incorrectly.
    void *dummy;
    gc_collect_start();
    gc_collect_root(&dummy, ((mp_uint_t)stack_top - (mp_uint_t)&dummy) / sizeof(mp_uint_t));
    gc_collect_end();
    gc_dump_info();
}

mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    return NULL;
}

mp_import_stat_t mp_import_stat(const char *path) {
    return MP_IMPORT_STAT_NO_EXIST;
}

mp_obj_t mp_builtin_open(uint n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);

void nlr_jump_fail(void *val) {
}

void NORETURN __fatal_error(const char *msg) {
    while (1);
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
#endif

