#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "net.h"

#define DEFAULT_PORT "8080"

int debug = 0;

void usage();
int is_number(char *str);

int main(int argc, char *argv[])
{
    char *cgi_dir = NULL;
    char *ip = NULL;
    char *logfile = NULL;
    char *port = DEFAULT_PORT;
    int opt;
    while ((opt = getopt(argc, argv, "c:dhi:l:p:")) != -1) {
        switch (opt) {
            case 'c':
                cgi_dir = optarg;
                break;
            case 'd':
                debug = 1;
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
                if (is_number(optarg))
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
    printf("cgi_dir = %s\n", cgi_dir);
    printf("logfile = %s\n", logfile);
    printf("ip = %s\n", ip);
    printf("port = %s\n==========\n", port);
    network_loop(ip, port);
    return 0;
}

void
usage()
{
    printf("sws [ -dh ] [ -c dir ] [ -i address ] [ -l file ] [ -p port ] dir\n");
}

int
is_number(char *str)
{
    int i;
    int len = strlen(str);
    for (i = 0; i < len; ++i)
        if (str[i] < '0' || str[i] > '9')
            return 0;
    if (atoi(str) > 65535)
        return 0;
    return 1;
}