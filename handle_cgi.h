#ifndef __HANDLE_CGI_H__
#define __HANDLE_CGI_H__

#include "public.h"

/*
 * Needn't allocate/free memory.
 */
int handle_cgi(/*Input*/_request *request, /*Output*/_response *response);

#endif /* end of include guard: __HANDLE_CGI_H__ */
