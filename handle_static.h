#ifndef __HANDLE_STATIC_H__
#define __HANDLE_STATIC_H__

#include "public.h"

#define MAX_TIME_SIZE 100
#define SERVER_NAME "sws"
#define SERVER_NAME_SIZE 3
#define VERSION "HTTP/1.0"
#define BUFF_SIZE 1024


/*
 * Needn't allocate/free memory.
 */
int handle_static(/*Input*/const _request *request, /*Output*/_response *response);
int if_modified(const _request *request, const struct stat req_stat)
int same_time(const char* val, const time_t mtime)
int set_file(const _request *request, const struct stat req_stat, _response *response)
int set_directory(_request *request, struct stat req_stat, _response *response)


#endif /* end of include guard: __HANDLE_STATIC_H__ */

