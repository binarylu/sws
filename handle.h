#ifndef __HANDLE_H__
#define __HANDLE_H__

#include <unistd.h>

#include <stdio.h>

#include "public.h"
#include "HTTP_parser.h"

#define BUF_SIZE 40960

int init_handle();
int handle(_client_info *client);
void destroy_handle();

#endif /* end of include guard: __HANDLE_H__ */
