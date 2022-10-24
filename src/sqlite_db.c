#include "sqlite_db.h"

int open_log(sqlite3 **db) {
    int rc = 0;
    char *err_msg = NULL;
    rc = sqlite3_open(LOG_PATH, db);
    if (rc != SQLITE_OK) 
    {
        syslog(LOG_ERR, "Can't open/create database");
        return -1;
    }

    char sql[128];

    sprintf(sql,"CREATE TABLE if NOT EXISTS %s(time TEXT, topic_name TEXT, msg TEXT);", TABLE_NAME);

    rc = sqlite3_exec(*db, sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK ) 
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);       
        return -1;
    }
    return 0;
}

int log_write(sqlite3 *db, char *current_time, char *topic_name, char *msg){

    int rc = 0;

    char *err_msg = 0;

    char sql[2000];

    sprintf(sql, "INSERT INTO %s VALUES('%s', '%s', '%s');", TABLE_NAME, current_time, topic_name, msg);

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK ) {
        
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        
        sqlite3_free(err_msg);        
        sqlite3_close(db);

        return rc;
    } 
    return rc;
}

void close_log(sqlite3 *db) {

if(sqlite3_close(db) != SQLITE_OK){
    syslog(LOG_ERR, "Couldn't close database");
}
return;
}

