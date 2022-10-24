#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <argp.h>
#include <string.h>
#include <stdlib.h>

typedef struct arguments
{
    char *args[5];
    char *hostname;
    int  port;
    char *user;
    char *pass;
    char *crt_path;
}arguments;

extern struct argp_option argp_options[];

error_t parse_opt(int key, char *arg, struct argp_state *state);

#endif