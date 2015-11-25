#ifndef __HANDLE_STATIC_H__
#define __HANDLE_STATIC_H__

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <dirent.h>
#include <magic.h>

#include "handle_response.h"


#define MAX_TIME_SIZE 100
#define SERVER_NAME "sws"
#define SERVER_NAME_SIZE 3
#define VERSION "HTTP/1.0"
#define BUFF_SIZE 1024
#define INDEX "index.html"


/*
 * Needn't allocate/free memory.
 */
int handle_static(/*Input*/_request *request, /*Output*/_response *response);
int if_modified(const _request *request, const struct stat* req_stat);
int same_time(const char* val, const time_t mtime);
int set_file(const _request *request, const struct stat* req_stat, _response *response);
int set_directory(_request *request, struct stat* req_stat, _response *response);
const char* getMIME(const char* path);


#endif /* end of include guard: __HANDLE_STATIC_H__ */

