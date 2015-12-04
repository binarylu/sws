#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "net.h"

#define DEFAULT_PORT "8080"

int g_debug = 0;
const char *g_log = NULL;
const char *g_dir = NULL;
const char *g_dir_cgi = NULL;

void usage();

int main(int argc, char *argv[])
{
    char *ip = NULL;
    char *port = DEFAULT_PORT;
    int opt;
    while ((opt = getopt(argc, argv, "c:dhi:l:p:")) != -1) {
        switch (opt) {
            case 'c': g_dir_cgi = optarg; break;
            case 'd': g_debug = 1; break;
            case 'h': usage(); exit(EXIT_SUCCESS); break;
            case 'i': ip = optarg; break;
            case 'l': g_log = optarg; break;
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
        fprintf(stderr, "Please provide dir!\n");
        exit(EXIT_FAILURE);
    }
    g_dir = argv[0];
    if (!validate_path(g_dir)) {
        fprintf(stderr, "%s is not a validated directory!\n", g_dir);
        exit(EXIT_FAILURE);
    }
    if (g_dir_cgi != NULL && !validate_path(g_dir_cgi)) {
        fprintf(stderr, "%s is not a validated CGI directory!\n", g_dir_cgi);
        exit(EXIT_FAILURE);
    }
    if (g_log != NULL && validate_path(g_log)) {
        fprintf(stderr, "%s is not a validated file!\n", g_log);
        exit(EXIT_FAILURE);
    }
    if (g_log != NULL)
        if (init_log(g_log) != 0) {
            fprintf(stderr, "Fail to open log file!");
            exit(EXIT_FAILURE);
        }
    if (!validate_port(port)) {
        fprintf(stderr, "Invalid port number!\n");
        exit(EXIT_FAILURE);
    }

    DEBUG("========== arguments =========");
    DEBUG("dir = %s", g_dir);
    DEBUG("cgi_dir = %s", g_dir_cgi);
    DEBUG("logfile = %s", g_log);
    DEBUG("ip = %s", ip);
    DEBUG("port = %s", port);

    if (g_debug == 0)
        if (daemon(1, 0) != 0) {
            FATAL_ERROR("Fail to daemon!");
        }

    network_loop(ip, port);

    return 0;
}

void
usage()
{
    printf("sws [ -dh ] [ -c dir ] [ -i address ] [ -l file ] [ -p port ] dir\n");
}
