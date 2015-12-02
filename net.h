#ifndef _NET_H_
#define _NET_H_

#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "public.h"
#include "handle.h"

#define MAX_LISTEN 5

#define SET_CONNECTION(c, _fd, _addr)                               \
do {                                                                \
    (c).fd = (_fd);                                                 \
    (c).buf = NULL;                                                 \
    (c).pos = 0;                                                    \
    memcpy((c).addr, &(_addr), sizeof(struct sockaddr_storage));    \
    request_init((c).request);                                      \
    response_init((c).response);                                    \
} while( /* CONSTCOND */ 0)

#define RESET_CONNECTION(c)                                 \
do {                                                        \
    (c).fd = -1;                                            \
    (c).buf = NULL;                                         \
    (c).pos = 0;                                            \
    memset((c).addr, 0, sizeof(struct sockaddr_storage));   \
    request_clear((c).request);                             \
    response_clear((c).response);                           \
} while( /* CONSTCOND */ 0)

/*
 * Memory should be allocated before calling
 */
int sockaddr2string(struct sockaddr *sa, char *address);
unsigned short int get_port(struct sockaddr *sa);
int validate_port(char *str);
int validate_ipv4(const char *ip);

void network_loop(char *address, char *port);

#endif /* !_NET_H_ */
