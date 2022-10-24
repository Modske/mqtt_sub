#ifndef SQLITE_DB_H
#define SQLITE_DB_H

#include <syslog.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#define LOG_PATH   "/var/log/mqtt_sub.db"
#define TABLE_NAME "mqttdata" 

int open_log(sqlite3 **db);
int log_write(sqlite3 *db, char *current_time, char *topic_name, char *msg);
void close_log(sqlite3 *db);

#endif 