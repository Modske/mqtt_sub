#include "arguments.h"

struct argp_option argp_options[] =
{
    {"hostname",    'h',  "HOSTNAME",    0, "broker hostname"},
    {"port"    ,    'l',  "PORT"    ,    0, "listening port"},
    {"username",    'u',  "USERNAME",    0, "provide username"},
    {"password",    'p',  "PASSWORD",    0, "provide password"},
    {"certificate", 'c',  "CERTIFICATE", 0, "provide path to a ca file"},
	{0}
};

error_t parse_opt(int key, char *arg, struct argp_state *state){
    arguments *arguments = state->input;
    switch(key)
    {
        case 'h': 
            arguments->hostname = arg;
            break;

        case 'l': 
            arguments->port = atoi(arg);
            break;
            
        case 'u': 
            arguments->user = arg;
            break;

        case 'p': 
            arguments->pass = arg;
            break;

        case 'c': 
            arguments->crt_path = arg;
            break;

		case ARGP_KEY_ARG:
			arguments->args[state->arg_num] = arg;
			break;
		case ARGP_KEY_END:
			if ((!arguments->hostname))
		{
			argp_usage(state);
		}
		break;
		default:
			return ARGP_ERR_UNKNOWN;
    }
	return 0;
}