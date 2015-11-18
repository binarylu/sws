#ifndef __HANDLE_STATIC_H__
#define __HANDLE_STATIC_H__

#include "public.h"

/*
 * Needn't allocate/free memory.
 */
int handle_static(/*Input*/const _request *request, /*Output*/_response *response);

#endif /* end of include guard: __HANDLE_STATIC_H__ */

