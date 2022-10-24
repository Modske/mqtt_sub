#include "mosq.h"

void con_handler(struct mosquitto *mosq, void *obj, int reason_code)
{
	int rc = 0;

	syslog(LOG_INFO,"on_connect: %s\n", mosquitto_connack_string(reason_code));

}

void sub_handler(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	int i;
	bool have_subscription = false;

	for(i=0; i<qos_count; i++){
		syslog(LOG_INFO, "on_subscribe: %d:granted qos = %d\n", i, granted_qos[i]);
		if(granted_qos[i] <= 2){
			have_subscription = true;
		}
	}
	if(have_subscription == false){

		syslog(LOG_ERR, "Error: All subscriptions rejected.\n");
		mosquitto_disconnect(mosq);
	}
}

void msg_handler(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{	
	int rc;

	time_t now;
    time(&now);
    char *current_time = ctime(&now);
    current_time[strcspn(current_time, "\n")] = 0;

	printf("Topic: %s. Message:%s\n", msg->topic, (char *)msg->payload);

	matched_events(topic_head, (char *)msg -> payload, msg -> topic, sender);

	log_write(db, current_time, msg->topic, (char *)msg->payload);
}

struct mosquitto* mosquitto_init(arguments *cli_args)
{
	int rc = 0;
	struct mosquitto *mosq;

	if((cli_args -> port == 0) || (cli_args -> hostname == NULL))
	{
		syslog(LOG_ERR, "Hostname or port wasn't provided\n");
        return NULL;
	}

	mosquitto_lib_init();
    
    mosq = mosquitto_new(NULL, true, NULL);
    if(mosq == NULL){
        syslog(LOG_ERR, "Error creating new mosquitto instance.\n");
        return NULL;
    }

    else
    {
        syslog(LOG_INFO, "Succesfully created a mosquitto instance");
    }

	mosquitto_connect_callback_set(mosq, con_handler);
	mosquitto_subscribe_callback_set(mosq, sub_handler);
	mosquitto_message_callback_set(mosq, msg_handler);

	if((cli_args -> user != NULL) && (cli_args -> pass != NULL))
	{
		rc = mosquitto_username_pw_set(mosq, cli_args -> user, cli_args -> pass);

		if(rc != MOSQ_ERR_SUCCESS)
			{
				syslog(LOG_ERR, "Mosquitto error: %s\n", mosquitto_strerror(rc));
				return NULL;
			}
		syslog(LOG_INFO, "Password set sucessfully");
	}

	if(cli_args -> crt_path != NULL)
	{
		rc = mosquitto_tls_set(mosq, cli_args -> crt_path, NULL,  NULL, NULL, NULL);
		if(rc != MOSQ_ERR_SUCCESS)
			{
				syslog(LOG_ERR, "Mosquitto error: %s\n", mosquitto_strerror(rc));
				return NULL;
			}
		syslog(LOG_INFO, "Certificate set succesfully");
	}

	return mosq;
}

int mosquitto_run(struct mosquitto *mosq, topic_node *topic_list, arguments *cli_args)
{
	int rc = MOSQ_ERR_SUCCESS; 

    rc = mosquitto_connect(mosq, cli_args -> hostname, cli_args -> port, 60);
	if(rc != MOSQ_ERR_SUCCESS)
	{
			syslog(LOG_ERR, "Mosquitto error: %s\n", mosquitto_strerror(rc));
			return rc;
	}

	topic_node *tmp_list = topic_list;

    while(tmp_list != NULL)
    {   
        rc = mosquitto_subscribe(mosq, NULL, tmp_list->topic, 1);
		if(rc != MOSQ_ERR_SUCCESS)
			{
				syslog(LOG_ERR, "Mosquitto error: %s\n", mosquitto_strerror(rc));
				return rc;
			}
        tmp_list = tmp_list->next;
    }

	rc = mosquitto_loop_start(mosq);

	if(rc != MOSQ_ERR_SUCCESS)
	{
		syslog(LOG_ERR, "Mosquitto error: %s\n", mosquitto_strerror(rc));
		return rc;
	}

	return rc;
}
