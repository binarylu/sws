#ifndef _PUBLIC_H_
#define _PUBLIC_H_

#include <sys/stat.h>

#include <netdb.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/*#define DEVELOPMENT*/
#define SERVER_NAME         "sws 1.0"
#define HTTP_VERSION        "HTTP/1.0"

extern int g_debug;
extern const char *g_log;
extern const char *g_dir;
extern const char *g_dir_cgi;

typedef enum { NONE_METHOD, GET_METHOD, HEAD_METHOD /*, POST_METHOD*/ } _method;
typedef enum { REQ_CGI, REQ_STATIC, REQ_OTHER } _request_type;
typedef enum { NO_ERR, REQ_LINE_ERR, FORMAT_ERR, SYSTEM_ERR } _request_err;

typedef struct _header_entry {
    char *key;
    char *value;
    struct _header_entry *next;
} _header_entry;

typedef struct _request {
    _method method;
    char *uri;
    char *version;
    _header_entry *header_entry;
    _request_err errcode;
} _request;

typedef struct _response {
    unsigned int code;
    char *desc;
    char *version;
    _header_entry *header_entry;
    char *body;
    int is_cgi;
} _response;

typedef struct _connection {
    int fd;
    char *buf;
    size_t pos;
    struct sockaddr_storage *addr;
    _request *request;
    _response *response;
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
/*void close_log();*/

void get_year_mon_day(int* year, int* mon, int* day);
void get_date_rfc1123(char *buf, size_t len);
void get_date_rfc850(char *buf, size_t len);
void get_date_asctime(char *buf, size_t len);

const char *seperate_string(const char *str, const char *delim,
        size_t *len, int idx);
int validate_path(const char *path);
int validate_path_security(const char *path, _request_type req_type, char **user_prefix);

/*
 * Caller is responsible to free the return memory
 */
char *get_absolute_path(const char *path, _request_type req_type, char **user_prefix);

#define LOG2FILE(fmt, arg...)               \
do {                                        \
    FILE *f = NULL;                         \
    if ((f = fopen(g_log, "a")) != NULL) {  \
        fprintf(f, fmt, ##arg);             \
        fclose(f);                          \
    }                                       \
} while( /* CONSTCOND */ 0)

#define LOG(fmt, arg...)                    \
do {                                        \
    if (g_debug == 1) {                     \
        fprintf(stdout, fmt"\n", ##arg);    \
        fflush(stdout);                     \
    } else if (g_log != NULL) {             \
        LOG2FILE(fmt"\n", ##arg);           \
    }                                       \
} while( /* CONSTCOND */ 0)

#define MSG(type, fmt, arg...)                      \
do {                                                \
    if (g_debug == 1) {                             \
        fprintf(stderr, #type": "fmt"\n", ##arg);   \
        fflush(stderr);                             \
    } else if (g_log != NULL) {                     \
        LOG2FILE(#type": "fmt"\n", ##arg);          \
    }                                               \
} while( /* CONSTCOND */ 0)

#define MSGP(type, fmt, arg...)                                         \
do {                                                                    \
    if (g_debug == 1) {                                                 \
        fprintf(stderr, #type": "fmt": %s\n", ##arg, strerror(errno));  \
        fflush(stderr);                                                 \
    } else if (g_log != NULL) {                                         \
        LOG2FILE(g_log, #type": "fmt": %s\n", ##arg, strerror(errno));  \
    }                                                                   \
} while( /* CONSTCOND */ 0)

#define FATAL_ERROR(fmt, arg...)    \
do {                                \
    MSGP(Error, fmt, ##arg);        \
    exit(EXIT_FAILURE);             \
} while( /* CONSTCOND */ 0)

#define ERROR(fmt, arg...)      \
do {                            \
    MSG(Error, fmt, ##arg);     \
    exit(EXIT_FAILURE);         \
} while( /* CONSTCOND */ 0)

#define RET_ERROR(retval, fmt, arg...)  \
do {                                    \
    MSG(Error, fmt, ##arg);             \
    return retval;                      \
} while( /* CONSTCOND */ 0)

#define RET_ERRORP(retval, fmt, arg...) \
do {                                    \
    MSGP(Error, fmt, ##arg);            \
    return retval;                      \
} while( /* CONSTCOND */ 0)

#define WARN(fmt, arg...)       \
do {                            \
    MSG(Warning, fmt, ##arg);   \
} while( /* CONSTCOND */ 0)

#define WARNP(fmt, arg...)      \
do {                            \
    MSGP(Warning, fmt, ##arg);  \
} while( /* CONSTCOND */ 0)

#ifdef DEVELOPMENT
#define DEBUG(fmt, arg...)      \
do {                            \
    MSG(Debug, fmt, ##arg);     \
} while( /* CONSTCOND */ 0)
#define DEBUGP(fmt, arg...)     \
do {                            \
    MSGP(Debug, fmt, ##arg);    \
} while( /* CONSTCOND */ 0)
#else
#define DEBUG(fmt, arg...)
#define DEBUGP(fmt, arg...)
#endif

#endif /* !_PUBLIC_H_ */
