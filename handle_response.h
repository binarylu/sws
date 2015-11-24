#ifndef __HANDLE_RESPONSE_H__
#define __HANDLE_RESPONSE_H__

#include "public.h"

void respond_not_found(_response *response); /* 404 */
void respond_forbidden(_response *response); /* 403 */
int generate_desc(_response* response);

#endif /* end of include guard: __HANDLE_RESPONSE_H__ */
