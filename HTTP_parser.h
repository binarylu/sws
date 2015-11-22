#ifndef __HTTP_PARSER_H__
#define __HTTP_PARSER_H__

#include "public.h"

/*
 * Needn't allocate/free memory.
 */
int decode_request(/*Input*/const char *content, /*Output*/_request *request);
char * encode_response(/*Input*/const _response *response);


#endif /* end of include guard: __HTTP_PARSER_H__ */
