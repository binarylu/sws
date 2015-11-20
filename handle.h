#ifndef __HANDLE_H__
#define __HANDLE_H__

#include <unistd.h>

#include <stdio.h>

#include "public.h"
#include "HTTP_parser.h"
#include "handle_static.h"
#include "handle_cgi.h"
#include "handle_other.h"

#define BUFFSIZE 40960

int init_handle();
int handle(_connection *connection);
void destroy_handle();

int validate_ipv4(const char *ip);
int validate_path(const char *path);
int validate_path_security(const char *path, _request_type req_type);

#endif /* end of include guard: __HANDLE_H__ */
