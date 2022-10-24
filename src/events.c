#include "events.h"

/**********************EMAILS*****************************/

static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;
  size_t room = size * nmemb;

  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }

  data = &upload_ctx->msg[upload_ctx->bytes_read];

  if(data) {
    size_t len = strlen(data);
    if(room < len)
      len = room;
    memcpy(ptr, data, len);
    upload_ctx->bytes_read += len;

    return len;
  }

  return 0;
}

char *recipient_string(email_node *emails)
{
    char *tmp_string, *email_string;
    email_string = strdup(emails -> address);
    email_string = strcat(email_string, "; ");

    email_node *tmp = emails -> next;

    while(tmp != NULL)
        {   
            tmp_string = strdup(tmp -> address);
            tmp_string = strcat(tmp_string, "; ");
            email_string = strcat(email_string, tmp_string);   
            tmp = tmp -> next;
        }
    return email_string;
}

int event_report(event_node *event, sender_info *sender)
{
    CURL *curl;
    CURLcode res = CURLE_OK;
    char *msg, *recipient_emails;
    struct curl_slist *recipients = NULL;

    email_node *emails = event -> emails;

    recipient_emails = recipient_string(emails);

    curl = curl_easy_init();

    if(curl)
    {   
        curl_easy_setopt(curl, CURLOPT_URL, sender -> host);

        if((sender -> cert_path)[0] == '\0')
        {   
            curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
            curl_easy_setopt(curl, CURLOPT_CAINFO, sender -> cert_path);
        }

        curl_easy_setopt(curl, CURLOPT_USERNAME, sender -> username);

        curl_easy_setopt(curl, CURLOPT_PASSWORD, sender -> pass);

        while(emails != NULL)
        {
            recipients = curl_slist_append(recipients, emails -> address);
            emails = emails -> next;
        }

        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        msg = (char*) malloc(sizeof(char) * (strlen(recipient_emails) +
                                             strlen(sender -> username) + 
                                             strlen(event -> topic) + 
                                             strlen(event -> name) + 
                                             strlen(event -> sign) + 
                                             strlen(event -> value) + 100));

        

        sprintf(msg,
            "To: <%s> \r\n"
            "From: <%s> \r\n"
            "Subject: %s\r\n"
            "\r\n" 
            "Topic: %s\r\n"
            "Received %s value %s %s\r\n"
            "\r\n", 
            recipient_emails, sender -> username, "Event report",
             event -> topic, event -> name, event -> sign, event -> value);

        struct upload_status data;

        data.msg = msg;

        data.bytes_read = 0;

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);

        curl_easy_setopt(curl, CURLOPT_READDATA, &data);

        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        /* Send the message */
        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
        {
            syslog(LOG_ERR, "curl_easy_perform() failed: %s\n", 
                   curl_easy_strerror(res));
        }

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);

        free(msg);
    }

    return 0;
}

/******************EVENT CHECKING*************************/

topic_node* find_topic(topic_node *head_node, char *topic)
{
    topic_node *tmp = head_node;

    while(strcmp(tmp -> topic, topic) != 0 )
    {
        tmp = tmp -> next;
    }

    return tmp;
}

int sign_check(char* sign)
{
    int int_sign = 0;

    if(strcmp(sign, ">") == 0)
        {
            int_sign = MORE;        
        }

        if(strcmp(sign, "<") == 0)
        {
            int_sign = LESS;        
        }

        if(strcmp(sign, "=") == 0)
        {
            int_sign = EQUAL;        
        }   

    return int_sign;
}

int digit_value_compare(int value, int threshold, int sign)
{   
    switch(sign)
    {
        case MORE:
            if(value > threshold)
            {
                return 0;
            }
            break;

        case LESS:
            if(value < threshold)
            {
                return 0;
            }
        break;

        case EQUAL:
            if(value = threshold)
            {
                return 0;
            }
        break;
    }
    return 1;
}

int string_value_compare(char* value, char* threshold)
{   
    int rc = 0;

    rc = strcmp(value, threshold);
                if(rc == 0)
                {
                  return 0;  
                }

    return 1;
}

event_node* check_event_match(event_node *event, struct json_object *obj)
{
    int rc = 0;

    struct json_object* key = json_object_object_get(obj, event -> name);

        if(key == NULL)
        {
            syslog(LOG_INFO, "Couldn't get parameter from msg\n");
            return NULL;
        }
    
    char *value = json_object_get_string(key);

    int sign = sign_check(event -> sign);

    if(strcmp(event -> type, "digit") == 0)
    {   
        if(digit_value_compare(atoi(value), atoi(event -> value), sign) == 0)
        {
            return event;
        }
    }

    if(strcmp(event -> type, "string") == 0)
    {
        if(string_value_compare(value, event -> value) == 0)
        {
            return event;
        }
    }


        syslog(LOG_ERR, "Provided wrong event type should be digit or string");

    return NULL;

}

void matched_events(topic_node *topic_head, char *msg, char *topic, sender_info *sender)
{   
    struct json_object *obj = NULL;
    int rc = 0;

    //Checking if received msg is in json format
    obj = json_tokener_parse(msg);
        if(obj == NULL)
        {
            syslog(LOG_INFO, "Received msg is not json format");
            return 1;
        }
    
    //Finding node that matches the received msg topic
    topic_node *matched_topic = NULL;
    matched_topic = find_topic(topic_head, topic);

    //Checking if there are events for this topic
        if(matched_topic -> events == NULL)
        {   
            syslog(LOG_INFO, "There are no events for this topic");
            return 1;
        }

    //Checking if any events are matched
    event_node *events = matched_topic -> events;
    event_node *matched_event = NULL;

        while(events != NULL)
        {   
            matched_event = check_event_match(events, obj);

                if(matched_event != NULL)
                {
                    //Actions to be performed if any of the events matched
                    rc = event_report(matched_event, sender);
                }
        
            events = events -> next;
        }
    return;
    
}