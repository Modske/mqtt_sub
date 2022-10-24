#ifndef EVENTS_H
#define EVENTS_H

#include <stdio.h>
#include <string.h>
#include "cfg_data.h"
#include <curl/curl.h>

#define LESS 10
#define MORE 11
#define EQUAL 12

void matched_events(topic_node *topic_head, char *msg, char *topic, sender_info *sender);

struct upload_status {
  size_t bytes_read;
  char *msg;
};

#endif