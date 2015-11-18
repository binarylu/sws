#ifndef __PUBLIC_H__
#define __PUBLIC_H__

#include <netdb.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef enum { NONE_METHOD, GET_METHOD, HEAD_METHOD /*, POST_METHOD*/ } _method;

typedef struct __header_entry {
    char *key;
    char *value;
    struct __header_entry *next;
} _header_entry;

typedef struct __request {
    _method method;
    char *uri;
    char *version;
    _header_entry *header_entry;
} _request;

typedef struct __response {
    unsigned int code;
    char *desc;
    char *version;
    _header_entry *header_entry;
} _response;

typedef struct __connection {
    int fd;
    char *buf;
    size_t pos;
    struct sockaddr_storage addr;
    _request request;
    _response response;
} _connection;

void request_init(_request *req);
void request_clear(_request *req);
/* return 0 on success */
int request_addfield(_request *req, const char *key,
        int keylen, const char *val, int vallen);

void response_init(_response *resp);
void response_clear(_response *resp);
/* return 0 on success */
int response_addfield(_response *resp, const char *key,
        int keylen, const char *val, int vallen);

#endif /* end of include guard: __PUBLIC_H__ */
