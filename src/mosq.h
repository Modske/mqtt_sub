#ifndef MOSQ_H
#define MOSQ_H

#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <time.h>

#include "main.h"
#include "events.h"
#include "sqlite_db.h"
#include "cfg_data.h"
#include "arguments.h"

void con_handler(struct mosquitto *mosq, void *obj, int reason_code);
void sub_handler(struct mosquitto *mosq, void *obj,
                 int mid, int qos_count, const int *granted_qos);
void msg_handler(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);
struct mosquitto* mosquitto_init(arguments *cli_args);
int mosquitto_run(struct mosquitto *mosq, topic_node *topic_list, arguments *cli_args);

#endif