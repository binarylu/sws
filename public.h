#ifndef __PUBLIC_H__
#define __PUBLIC_H__

#include <netdb.h>

typedef struct __client_info {
    int fd;
    struct sockaddr_storage addr;
} _client_info;

typedef enum { NONE_METHOD, GET_METHOD, HEAD_METHOD /*, POST_METHOD*/ } _method;

typedef struct __header {
    char *key;
    char *value;
    struct __header *next;
} _header;

typedef struct __request {
    _method method;
    char *uri;
    char *version;
    _header *head_entry;
} _request;

/* request operations */
/* initialize struct */
void request_init(_request *req);
/* release memory and reset */
void request_clear(_request *req);
/* return 0 on success */
int request_addfield(_request *req, const char *key,
        int keylen, const char *val, int vallen);

typedef struct __response {
    unsigned int code;
    char *desc;
    char version[9];
    _header *head_entry;
} _response;

#endif /* end of include guard: __PUBLIC_H__ */
