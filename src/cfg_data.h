#ifndef CFG_DATA_H
#define CFG_DATA_H

#include <syslog.h>
#include <uci.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PACKAGE_NAME "mqtt_sub"

//Topic parsing
#define TOPIC_SECTION_TYPE "topic"       
#define TOPIC_OPTION_NAME  "name"

//Event parsing
#define EVENT_SECTION_TYPE "event"
#define EVENT_OPTION_TOPIC "topic"
#define EVENT_OPTION_NAME  "key"
#define EVENT_OPTION_TYPE  "type"
#define EVENT_OPTION_SIGN  "compare_sign"
#define EVENT_OPTION_VALUE "compare_value"
#define EVENT_EMAIL_LIST   "email"

//Sender email info
#define SENDER_SECTION_TYPE "sender"
#define SENDER_OPTION_USER  "username"
#define SENDER_OPTION_PASS  "password"
#define SENDER_OPTION_HOST  "hostname"
#define SENDER_OPTION_CERT  "cert"

typedef struct
{
    char username[128];
    char pass[128];
    char host[128];
    char cert_path[512];
}sender_info;

typedef struct
{
    char topic[128];
    struct event_node *events;
    struct topic_node *next;

} topic_node;

typedef struct 
{   
    char topic[128];
    char name[64];
    char type[64];
    char sign[10];
    char value[64];
    struct email_node *emails;
    struct event_node *next;
}event_node;

typedef struct
{
    char address[64];
    struct email_node *next;
}email_node;

int uci_get_topics(topic_node **topics);
int uci_get_events(topic_node **topics);
int uci_get_sender_info(sender_info **info);
topic_node* create_topic_node(char* data);
event_node* create_event_node();
email_node* create_email_node(char* address);
void add_to_topic_list(topic_node **list, topic_node *node);
void add_to_event_list(event_node **list, event_node *node);
void add_to_email_list(email_node **list, email_node *node);
void delete_list(topic_node **list);

#endif