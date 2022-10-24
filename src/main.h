#ifndef MAIN_H
#define MAIN_H

#include <signal.h>
#include <unistd.h>
#include <mosquitto.h>
#include <sqlite3.h>

#include "mosq.h"
#include "cfg_data.h"
#include "sqlite_db.h"
#include "arguments.h"

extern sqlite3 *db;
extern topic_node *topic_head;
extern sender_info *sender;

#endif