#ifndef _HANDLE_CGI_H_
#define _HANDLE_CGI_H_

#include <sys/wait.h>

#include "public.h"

#define MAX_TIME_SIZE       100

/*
 * Needn't allocate/free memory.
 */
int handle_cgi(/*Input*/const _request *request, /*Output*/_response *response);

#endif /* !_HANDLE_CGI_H_ */
