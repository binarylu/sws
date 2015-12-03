#ifndef _HANDLE_STATIC_H_
#define _HANDLE_STATIC_H_

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#ifndef __APPLE__
#include <magic.h>
#endif

#include "handle_response.h"
#include "index.h"

#define MAX_TIME_SIZE       100
#define BUFF_SIZE           1024
#define INDEX               "index.html"

/*
 * Needn't allocate/free memory.
 */
int handle_static(/*Input*/_request *request, /*Output*/_response *response);
int handle_static_helper(/*Input*/_request *request, /*Output*/_response *response);
int if_modified(const _request *request, const struct stat* req_stat);
int same_time(const char* val, const time_t mtime);
int set_file(const _request *request, const struct stat* req_stat, _response *response);
int set_directory(_request *request, struct stat* req_stat, _response *response);
char* getMIME(const char* path);


#endif /* !_HANDLE_STATIC_H_ */

