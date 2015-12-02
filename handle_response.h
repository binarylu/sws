#ifndef _HANDLE_RESPONSE_H_
#define _HANDLE_RESPONSE_H_

#include "public.h"

void respond_not_found(_response *response); /* 404 */
void respond_forbidden(_response *response); /* 403 */
int generate_desc(_response* response);
char* generate_str(const char* desc);
int handleError(_response *response);

#endif /* !_HANDLE_RESPONSE_H_ */
