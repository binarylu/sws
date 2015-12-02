#ifndef _HTTP_PARSER_H_
#define _HTTP_PARSER_H_

#include "public.h"

/*
 * Needn't allocate/free memory.
 */
int decode_request(/*Input*/const char *content, /*Output*/_request *request);
char * encode_response(/*Input*/const _response *response);

#endif /* !_HTTP_PARSER_H_ */
