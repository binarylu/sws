#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "net.h"

#define DEFAULT_IP   "0.0.0.0"
#define DEFAULT_PORT "8080"

int g_debug = 0;
int g_logfd = -1;

void usage();

int main(int argc, char *argv[])
{
    char *cgi_dir = NULL;
    char *ip = DEFAULT_IP;
    char *logfile = NULL;
    char *port = DEFAULT_PORT;
    int opt;
    while ((opt = getopt(argc, argv, "c:dhi:l:p:")) != -1) {
        switch (opt) {
            case 'c':
                cgi_dir = optarg;
                break;
            case 'd':
                g_debug = 1;
                break;
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
                break;
            case 'i':
                ip = optarg;
                break;
            case 'l':
                logfile = optarg;
                break;
            case 'p':
                if (validate_port(optarg))
                    port = optarg;
                else {
                    fprintf(stderr, "Invalid port number\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case '?':
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }
    if (logfile != NULL)
        init_log(logfile);
    printf("cgi_dir = %s\n", cgi_dir);
    printf("logfile = %s\n", logfile);
    printf("ip = %s\n", ip);
    printf("port = %s\n==========\n", port);
    network_loop(ip, port);
    if (logfile != NULL)
        close_log();
    return 0;
}

void
usage()
{
    printf("sws [ -dh ] [ -c dir ] [ -i address ] [ -l file ] [ -p port ] dir\n");
}
