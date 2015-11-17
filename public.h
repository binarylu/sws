#ifndef __PUBLIC_H__
#define __PUBLIC_H__

#include <netdb.h>

typedef struct __client_info {
    int fd;
    struct sockaddr_storage addr;
} _client_info;

typedef enum {GET, HEAD, POST} _method;

typedef struct __header {
    char *key;
    char *value;
    struct __header *next;
} _header;

typedef struct __request {
    _method method;
    char *uri;
    char version[9];
    _header *head_entry;
} _request;

typedef struct __response {
    unsigned int code;
    char *desc;
    char version[9];
    _header *head_entry;
} _response;

#endif /* end of include guard: __PUBLIC_H__ */
