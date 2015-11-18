#ifndef __PUBLIC_H__
#define __PUBLIC_H__

#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

extern int g_debug;
extern int g_logfd;

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

int init_log(const char *filename);
void close_log();

void get_year_mon_day(int* year, int* mon, int* day);
void get_date_rfc1123(char *buf, size_t len);
void get_date_rfc850(char *buf, size_t len);
void get_date_asctime(char *buf, size_t len);

const char * seperate_string(const char *str, const char *delim,
        size_t *len, int idx);
int validate_ipv4(const char *ip);

#define LOG(fmt, arg...) do { \
    if (g_debug == 1) \
        fprintf(stdout, fmt, ##arg); \
    else if (g_logfd != -1) { \
        char msg[128]; \
        snprinf(msg, 128, fmt, ##arg); \
        write(g_fd_log, msg, 128); \
    } \
} while(0)

#endif /* end of include guard: __PUBLIC_H__ */
