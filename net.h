#ifndef __NET_H__
#define __NET_H__

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "public.h"
#include "handle.h"

#define MAX_LISTEN 5

#define SET_CONNECTION(c, _fd, _addr) do { \
    (c).fd = (_fd); \
    (c).buf = NULL; \
    (c).pos = 0; \
    (c).addr = (_addr); \
} while(0)

#define RESET_CONNECTION(c) do { \
    (c).fd = -1; \
    (c).buf = NULL; \
    (c).pos = 0; \
} while(0)

int sockaddr2string(struct sockaddr *sa, char *address);
unsigned short int get_port(struct sockaddr *sa);
int validate_port(char *str);
int validate_ipv4(const char *ip);

void network_loop(char *address, char *port);


#endif /* end of include guard: __NET_H__ */
