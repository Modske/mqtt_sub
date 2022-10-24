#include "cfg_data.h"

static topic_node* find_by_topic(topic_node *head_node, char *topic_name)
{
    topic_node *tmp = head_node;

    while(strcmp(tmp -> topic, topic_name) != 0)
    {
        tmp = tmp -> next;

        if(tmp == NULL)
        {   
            syslog(LOG_ERR, "There are no topics matching this event\n");
            return NULL;
        }
    }

    return tmp;
}

int uci_get_sender_info(sender_info **info)
{   
    sender_info *tmp = NULL;
    tmp = (sender_info*) malloc(sizeof(sender_info));

    if(tmp == NULL)
    {
        syslog(LOG_ERR, "Couldn't allocate memory for sender info");
        return -1;
    }

    int rc = 0;
    struct uci_context *ctx = NULL;
    struct uci_package *pkg;
    struct uci_element *e1, *e2;

    ctx = uci_alloc_context();
    if(ctx == NULL)
    {
        syslog(LOG_ERR, "Failed to allocate uci context");
        return -1;
    }

    rc = uci_load(ctx, PACKAGE_NAME, &pkg);
    if(rc != 0 || pkg == NULL)
    {
        syslog(LOG_ERR, "Failed to load uci package\n");
        uci_free_context(ctx);
        return -1;
    }

    uci_foreach_element(&pkg -> sections, e1)
    {
        struct uci_section *sec = uci_to_section(e1);

        if(strcmp(sec -> type, SENDER_SECTION_TYPE) == 0)
        {
            uci_foreach_element(&sec -> options, e2)
            {
                struct uci_option *opt = uci_to_option(e2);

                if(strcmp(opt -> e.name, SENDER_OPTION_USER) == 0)
                {
                    strcpy(tmp -> username, opt -> v.string);
                }

                if(strcmp(opt -> e.name, SENDER_OPTION_PASS) == 0)
                {
                    strcpy(tmp -> pass, opt -> v.string);
                }

                if(strcmp(opt -> e.name, SENDER_OPTION_HOST) == 0)
                {
                    strcpy(tmp -> host, opt -> v.string);
                }

                if(strcmp(opt -> e.name, SENDER_OPTION_CERT) == 0)
                {
                    strcpy(tmp -> cert_path, opt -> v.string);
                }
            }
        }
    }


    if(((tmp -> username)[0] == '\0') || (tmp -> pass)[0] == NULL || (tmp -> host)[0] == NULL)
    {
        syslog(LOG_ERR, "Couldn't get sender info \n");
        return -1;
    }
    
    *info = tmp;

    return 0;
}

int uci_get_topics(topic_node **topics)
{   
    int rc = 0;
    struct uci_context *ctx = NULL;
    struct uci_package *pkg;
    struct uci_element *e1;
    struct uci_option *topic_opt;

    ctx = uci_alloc_context();
    if(ctx == NULL)
    {
        syslog(LOG_ERR, "Failed to allocate uci context");
        return -1;
    }

    rc = uci_load(ctx, PACKAGE_NAME, &pkg);
    if(rc != 0 || pkg == NULL)
    {
        syslog(LOG_ERR, "Failed to load uci package\n");
        uci_free_context(ctx);
        return -1;
    }

    uci_foreach_element(&pkg->sections, e1)
    {
        struct uci_section *topic_section = uci_to_section(e1);
        if(strcmp(topic_section -> type, TOPIC_SECTION_TYPE) == 0)
            {
                topic_opt = uci_lookup_option(ctx, topic_section, TOPIC_OPTION_NAME);
                if(topic_opt == NULL)
                    {
                        syslog(LOG_ERR, "Failed to lookup option\n");
                        goto uci_list_err;
                    }
                
                if(!topic_opt || topic_opt->type != UCI_TYPE_STRING)
                {   
                    syslog(LOG_ERR, "Parsed option is not a string!\n");
                    goto uci_list_err;
                }
                
                topic_node *node = create_topic_node(topic_opt->v.string);

                if(node == NULL)
                {
                    syslog(LOG_ERR, "Topic node was not created\n");
                    goto uci_list_err;
                }

                add_to_topic_list(topics, node);
            }

        if(topics == NULL)
        {
            syslog(LOG_ERR, "No topics were provided\n");
            goto uci_list_err;
        }

    }

    uci_unload(ctx,pkg);
    uci_free_context(ctx);
    return 0;

    uci_list_err:
    uci_unload(ctx,pkg);
    uci_free_context(ctx);
    return -1;

}

int uci_get_events(topic_node **topics)
{   
    if(*topics == NULL)
    {
        syslog(LOG_ERR, "There are no registered topics!\n");
        return -1;
    }
    
    int rc = 0;
    struct uci_context *ctx = NULL;
    struct uci_package *pkg;
    struct uci_element *e1, *e2, *e3;
    struct uci_option *event_opt;

    ctx = uci_alloc_context();
    if(ctx == NULL)
    {
        syslog(LOG_ERR, "Failed to allocate uci context");
        return -1;
    }

    rc = uci_load(ctx, PACKAGE_NAME, &pkg);
    if(rc != 0 || pkg == NULL)
    {
        syslog(LOG_ERR, "Failed to load uci package\n");
        uci_free_context(ctx);
        return -1;
    }

    uci_foreach_element(&pkg -> sections, e1)
    {
        struct uci_section *event_section = uci_to_section(e1);
        if(strcmp(event_section -> type, EVENT_SECTION_TYPE) == 0)
        {   
            event_node *ev_node = create_event_node();

            if(ev_node == NULL)
            {
                syslog(LOG_ERR, "Failed to create event node\n");
                goto uci_list_err;
            }

            uci_foreach_element(&event_section -> options, e2)
            {
                struct uci_option *event_option = uci_to_option(e2);
                if(strcmp(event_option -> e.name, EVENT_OPTION_TOPIC) == 0)
                {
                    strcpy(ev_node -> topic, event_option -> v.string);
                }

                if(strcmp(event_option -> e.name, EVENT_OPTION_NAME) == 0)
                {
                    strcpy(ev_node -> name, event_option -> v.string);
                }

                if(strcmp(event_option -> e.name, EVENT_OPTION_TYPE) == 0)
                {
                    strcpy(ev_node -> type, event_option -> v.string);
                }

                if(strcmp(event_option -> e.name, EVENT_OPTION_SIGN) == 0)
                {
                    strcpy(ev_node -> sign, event_option -> v.string);
                }

                if(strcmp(event_option -> e.name, EVENT_OPTION_VALUE) == 0)
                {
                    strcpy(ev_node -> value, event_option -> v.string);
                }

                if(event_option->type == UCI_TYPE_LIST)
                {
                    uci_foreach_element(&event_option -> v.list, e3)
                    {   
                        email_node *em_node = create_email_node(e3 -> name);

                        if(em_node == NULL)
                        {
                            syslog(LOG_ERR, "Failed to create email node\n");
                            goto uci_list_err;
                        }
                        add_to_email_list(&(ev_node -> emails), em_node);
                    }
                }

            }

        topic_node *temp = NULL;
        temp = find_by_topic(*topics, ev_node -> topic);

        if(temp != NULL)
        {
            add_to_event_list(&(temp -> events), ev_node);
        }

        else
        {
            free(ev_node);
        }

        }

    }

    uci_unload(ctx,pkg);
    uci_free_context(ctx);
    return 0;

    uci_list_err:
    uci_unload(ctx,pkg);
    uci_free_context(ctx);
    return -1;
}

topic_node* create_topic_node(char* data)
{   
    topic_node *l = NULL;
    l = (topic_node*) malloc(sizeof(topic_node));

    if(l == NULL)
    {
        syslog(LOG_ERR, "Couldn't allocate memory for node");
        return NULL;
    }

        l->next = NULL;
        l->events=NULL;
        strcpy(l->topic, data);

    return l;
}

event_node* create_event_node()
{   
    event_node *l = NULL;
    l = (event_node*) malloc(sizeof(event_node));

    if(l == NULL)
    {
        syslog(LOG_ERR, "Couldn't allocate memory for node");
        return NULL;
    }

        l->next = NULL;
        l->emails=NULL;

    return l;
}

email_node* create_email_node(char *address)
{   
    email_node *l = NULL;
    l = (email_node*) malloc(sizeof(email_node));

    if(l == NULL)
    {
        syslog(LOG_ERR, "Couldn't allocate memory for node");
        return NULL;
    }

        l->next = NULL;
        strcpy(l -> address, address);

    return l;
}

void add_to_topic_list(topic_node **list, topic_node *node)
{   
    int rc = 0;

    topic_node* temp = *list;
    if (temp == NULL) {
        *list = node; 
        return;
    }

    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = node;
}

void add_to_event_list(event_node **list, event_node *node)
{   
    int rc = 0;

    event_node* temp = *list;
    if (temp == NULL) {
        *list = node; 
        return;
    }

    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = node;
}

void add_to_email_list(email_node **list, email_node *node)
{   
    int rc = 0;

    email_node* temp = *list;
    if (temp == NULL) {
        *list = node; 
        return;
    }

    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = node;
}

void delete_list(topic_node **list)
{   
    event_node *event_list = NULL;
    email_node *email_list = NULL;

    topic_node *topic_tmp = *list;
    event_node *event_tmp = NULL;
    email_node *email_tmp = NULL;

    //Freeing topic linked list
    while ((*list) != NULL) 
    {
        if((*list) -> events != NULL)
        {
            event_list = (*list) -> events;
            event_tmp = event_list;
        }
        //Freeing event linked list
        while(event_list != NULL)
        {
            email_list = event_list -> emails;
            email_tmp = email_list;
        //Freeing the email linked list
            while (email_list != NULL) 
            {
                email_list = email_list -> next;
                free(email_tmp);
                email_tmp = email_list;
            }

            event_list = event_list -> next;
            free(event_tmp);
            event_tmp = event_list;
        }

        *list = (*list) -> next;
        free(topic_tmp);
        topic_tmp = *list;
    }
}
