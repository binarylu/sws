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
#define BUFFSIZE 128

extern int sockaddr2string(struct sockaddr *sa, char *address);

extern void network_loop(char *address, char *port);


#endif /* end of include guard: __NET_H__ */
