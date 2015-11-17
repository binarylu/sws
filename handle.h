#ifndef __HANDLE_H__
#define __HANDLE_H__

#include <unistd.h>

#include <stdio.h>

#include "public.h"

#define BUF_SIZE 4096

int handle(_client_info *client);

#endif /* end of include guard: __HANDLE_H__ */
