#ifndef __HTTP_PARSER_H__
#define __HTTP_PARSER_H__

#include "public.h"

/*
 * Needn't allocate/free memory.
 */
int decode_request(/*Input*/const char *content, /*Output*/_request *request);
int encode_response(/*Input*/_response *response, /*Output*/char *content);


#endif /* end of include guard: __HTTP_PARSER_H__ */
