#ifndef _HANDLE_H_
#define _HANDLE_H_

#include <unistd.h>

#include <stdio.h>

#include "public.h"
#include "HTTP_parser.h"
#include "handle_static.h"
#include "handle_cgi.h"

#define BUFFSIZE 40960

int handle(_connection *connection);

#endif /* !_HANDLE_H_ */
