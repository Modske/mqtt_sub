#include "main.h"

//Global declarations
volatile int daemon_run = 1;
sqlite3 *db;
topic_node *topic_head;
sender_info *sender;
struct argp argp = {argp_options, parse_opt};

//Signal handler function declaration
void sigHandler(int sig);

int main(int argc, char **argv){
    struct sigaction action;
    struct mosquitto *mosq = NULL;
    int rc = 0, i = 0;

    //Signal handling
    action.sa_handler = sigHandler;
    sigfillset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    sigaction(SIGTERM, &action, NULL);

    //Argument parse
    arguments arguments;
    arguments.hostname = NULL;
    arguments.port = 0;
    arguments.user = NULL;
    arguments.pass = NULL;
    arguments.crt_path = NULL;

    argp_parse (&argp, argc, argv, 0, 0, &arguments);

    //Config parse and conversion to linked list
    rc = uci_get_topics(&topic_head);
    if(rc != 0)
        {   
            goto delete_linked_list;
        }

    rc = uci_get_events(&topic_head);
    if(rc != 0)
        {   
            goto delete_linked_list;
        }

    rc = uci_get_sender_info(&sender);
    if(rc != 0)
        {
            goto free_sender_info;
        }

    //SQLite db init 
    rc = open_log(&db);
    if(rc != 0)
        {
            goto close_database;
        }
    
    //Mosquitto new instance, callback registrations and connection

    mosq = mosquitto_init(&arguments);
    if(mosq == NULL)
        {
            goto mosquitto_cleanup;
        }

    rc = mosquitto_run(mosq, topic_head, &arguments);
    if(rc != MOSQ_ERR_SUCCESS)
        {
            goto mosquitto_cleanup;
        }

    while(daemon_run)
        {       
            sleep(1);
        }
    
    //Cleanup
    mosquitto_loop_stop(mosq, true);
    mosquitto_lib_cleanup();
    close_log(db);
    delete_list(&topic_head);
    return 0;
    
    mosquitto_cleanup:
    mosquitto_lib_cleanup();

    close_database:
    close_log(db);

    free_sender_info:
    free(sender);

    delete_linked_list:
    delete_list(&topic_head);
    return -1;

}


void sigHandler(int sig) 
{
    if(sig == SIGINT){
        syslog(LOG_INFO,"Received signal: %d", sig);
        daemon_run = 0;
    }
    return;
}