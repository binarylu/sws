#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "net.h"

#define DEFAULT_IP   "0.0.0.0"
#define DEFAULT_PORT "8080"

int g_debug = 0;
FILE *g_log = NULL;
const char *g_dir = NULL;
const char *g_dir_cgi = NULL;

void usage();

int main(int argc, char *argv[])
{
    char *ip = DEFAULT_IP;
    char *logfile = NULL;
    char *port = DEFAULT_PORT;
    int opt;
    while ((opt = getopt(argc, argv, "c:dhi:l:p:")) != -1) {
        switch (opt) {
            case 'c': g_dir_cgi = optarg; break;
            case 'd': g_debug = 1; break;
            case 'h': usage(); exit(EXIT_SUCCESS); break;
            case 'i': ip = optarg; break;
            case 'l': logfile = optarg; break;
            case 'p': port = optarg; break;
            case '?':
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }
    argc -= optind;
    argv += optind;
    if (argc == 0) {
        fprintf(stderr, "Please provide dir!");
        exit(EXIT_FAILURE);
    }
    g_dir = argv[0];
    if (!validate_path(g_dir)) {
        fprintf(stderr, "%s is not a validated path!", g_dir);
        exit(EXIT_FAILURE);
    }
    if (!validate_path(g_dir_cgi)) {
        fprintf(stderr, "%s is not a validated path!", g_dir_cgi);
        exit(EXIT_FAILURE);
    }
    if (!validate_port(port)) {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }

    printf("dir = %s\n", g_dir);
    printf("cgi_dir = %s\n", g_dir_cgi);
    printf("logfile = %s\n", logfile);
    printf("ip = %s\n", ip);
    printf("port = %s\n==========\n", port);
    exit(EXIT_FAILURE);

    if (g_debug == 0)
        if (daemon(0, 0) != 0) {
            perror("Fail to daemon!");
            exit(EXIT_FAILURE);
        }

    if (logfile != NULL)
        init_log(logfile);

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
