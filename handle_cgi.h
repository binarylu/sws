#ifndef _HANDLE_CGI_H_
#define _HANDLE_CGI_H_

#include <sys/wait.h>

#include "public.h"

/*
 * Needn't allocate/free memory.
 */
int handle_cgi(/*Input*/const _request *request, /*Output*/_response *response);

#endif /* !_HANDLE_CGI_H_ */
