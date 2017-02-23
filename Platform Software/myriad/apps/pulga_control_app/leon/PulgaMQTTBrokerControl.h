#include <stdio.h>
#include "linkedlist.h"
#include "MQTTPacket.h"
#include "camera.h"

#include <assert.h>
#include <SDCardIO.h>
#include <FlashIO.h>
#include <WifiFunctions.h>
#include <TimeFunctions.h>

#ifndef PULGAMQTTBROKERIMAGETEST_H
#define PULGAMQTTBROKERIMAGETEST_H

#define PORT_NUM 1883
#define BUF_SIZE 1472
#define ERROR_FRC -1
#define ERROR_DISCONNECTION 10

#define min(a, b) (((a) < (b)) ? (a) : (b)) 

typedef struct {
    char id[50];
    linkedlist *suscribed;
    linkedlist *queued_messages;
} topic_subscriptions;

typedef struct {
    char* name;
    linkedlist *connected_clients;
    linkedlist *topics_subscribers;
} pulgamqttbroker;


//METHODS

//start MQTT server
pulgamqttbroker *pulga_create();
int pulga_start(pulgamqttbroker *);
int transport_getdatanb(void *sck, unsigned char* buf, int count);
int transport_getdata(unsigned char* buf, int count, int socket);
int comparetopic_subscriptionsStruct(void *item1, void *item2);
int *new_int(int value);
int compare_int(void *item1, void *item2);
void to_string_print_int(FILE * output, void *item);
int publish_parser(pulgamqttbroker *, unsigned int, unsigned char, int, unsigned char, unsigned short, MQTTString, unsigned char*, int);
char** parse_args(int* variables, int max_args, unsigned char* payload, int payloadlen);
int publishToSubscribers(pulgamqttbroker *pulga, topic_subscriptions *aux_t, unsigned char dup, int qos, unsigned char retained, unsigned short packetid, MQTTString topicName, unsigned char* payload, int payloadlen);
int calculate_rec_packet(unsigned char* payload,int payloadlen,int chars_number_of_packet, MQTTString* topicName2);
unsigned char** receive_file(pulgamqttbroker *pulga, topic_subscriptions *aux_t, int number_of_packets, int payload_max, int chars_number_of_packet, unsigned int s, int* payload_image_size, unsigned char* dup2, int* qos2, unsigned char* retained2, unsigned short* packetid2, MQTTString* topicName2);
int send_file(pulgamqttbroker *pulga, topic_subscriptions *aux_t, unsigned int s, int sleep_packages, unsigned char** payload_image, int number_of_packets, int payload_max, int payload_image_size, unsigned char dup2, int qos2, unsigned char retained2, unsigned short packetid2, MQTTString topicName2);
int count_length_number(int num);
int send_snapshot(pulgamqttbroker *pulga, topic_subscriptions *aux_t, unsigned int s, int sleep_packages, int payload_max, unsigned char dup2, int qos2, unsigned char retained2, unsigned short packetid2, MQTTString topicName2);

#endif
