#ifndef __HANDLE_H__
#define __HANDLE_H__

#include <unistd.h>

#include <stdio.h>

#include "public.h"
#include "HTTP_parser.h"
#include "handle_static.h"
#include "handle_cgi.h"
#include "handle_other.h"

#define BUF_SIZE 40960

int init_handle();
int handle(_connection *connection);
void destroy_handle();

#endif /* end of include guard: __HANDLE_H__ */
