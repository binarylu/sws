#include "public.h"
#include <bsd/string.h>

static void
free_headers(_header_entry *p)
{
    _header_entry *prev;
    while (p) {
        prev = p;
        p = p->next;
        if (prev->key != NULL)
            free(prev->key);
        if (prev->value != NULL)
            free(prev->value);
        free(prev);
    }
}

void
request_init(_request *req)
{
    req->method = NONE_METHOD;
    req->uri = NULL;
    req->version = NULL;
    req->header_entry = NULL;
}

void
request_clear(_request *req)
{
    if (req->uri != NULL)
        free(req->uri);
    if (req->version != NULL)
        free(req->version);
    if (req->header_entry != NULL)
        free_headers(req->header_entry);
    request_init(req);
}

/*
 * return 0 on success
 */
int
request_addfield(_request *req, const char *key,
        int keylen, const char *val, int vallen)
{
    _header_entry *last;
    _header_entry *node;

    node = (_header_entry *)malloc(sizeof(_header_entry));
    if (node == NULL)
        return -1;

    if ((node->key = (char *)malloc(keylen + 1)) == NULL) {
        free(node);
        return -1;
    }
    if ((node->value = (char *)malloc(vallen + 1)) == NULL) {
        free(node->key);
        free(node);
        return -1;
    }

    assert(strlcpy(node->key, key, keylen + 1));
    assert(strlcpy(node->value, val, vallen + 1));

    last = req->header_entry;
    if (last == NULL)
        req->header_entry = node;
    else {
        while (last->next)
            last = last->next;
        last->next = node;
    }

    return 0;
}

void
response_init(_response *resp)
{
    resp->code = -1;
    resp->desc = NULL;
    resp->version = NULL;
    resp->header_entry = NULL;
}

void
response_clear(_response *resp)
{
    if (resp->desc != NULL)
        free(resp->desc);
    if (resp->version != NULL)
        free(resp->version);
    if (resp->version != NULL)
        free(resp->body);
    if (resp->header_entry != NULL)
        free_headers(resp->header_entry);
    response_init(resp);
}

/*
 * return 0 on success
 */
int
response_addfield(_response *resp, const char *key,
        int keylen, const char *val, int vallen)
{
    _header_entry *last;
    _header_entry *node;

    node = (_header_entry *)malloc(sizeof(_header_entry));
    if (node == NULL)
        return -1;

    if ((node->key = (char *)malloc(keylen + 1)) == NULL) {
        free(node);
        return -1;
    }
    if ((node->value = (char *)malloc(vallen + 1)) == NULL) {
        free(node->key);
        free(node);
        return -1;
    }

    assert(strlcpy(node->key, key, keylen + 1));
    assert(strlcpy(node->value, val, vallen + 1));

    last = resp->header_entry;
    if (last == NULL)
        resp->header_entry = node;
    else {
        while (last->next)
            last = last->next;
        last->next = node;
    }

    return 0;
}

int
init_log(const char *filename)
{
    if ((g_log = fopen(filename, "a")) == NULL) {
        perror("Fail to open log file");
        return -1;
    }
    return 0;
}

void
close_log()
{
    fclose(g_log);
}

void
get_year_mon_day(int* year, int* mon, int* day)
{
    time_t timep;
    struct tm *p;

    time(&timep);
    p = localtime(&timep);
    *year = p->tm_year + 1900;
    *mon = p->tm_mon + 1;
    *day = p->tm_mday;
}

void
get_date_rfc1123(char *buf, size_t len)
{
      time_t timep;
      struct tm *p;

      time(&timep);
      p = localtime(&timep);
      strftime(buf, len, "%a, %d %b %Y %H GMT", p);
}

void
get_date_rfc850(char *buf, size_t len)
{
      time_t timep;
      struct tm *p;

      time(&timep);
      p = localtime(&timep);
      strftime(buf, len, "%A, %d-%b-%y %H GMT", p);
}

void
get_date_asctime(char *buf, size_t len)
{
      time_t timep;
      struct tm *p;

      time(&timep);
      p = localtime(&timep);
      strftime(buf, len, "%c", p);
}

const char *
seperate_string(const char *str, const char *delim, size_t *len, int idx)
{
    int i = 0;
    const char *pre = str;
    const char *cur = str;

    if (str != NULL) {
        cur = strstr(pre, delim);
        for (i = 1; i <= idx && cur; ++i) {
            pre = cur + strlen(delim);
            cur = strstr(pre, delim);
        }
        if (cur != NULL) { /* There are more delims than idx */
            *len = cur - pre;
            return pre;
        } else if (i == 0 || i > idx) { /* The section is the only one or the last one */
            *len = strlen(pre);
            return pre;
        } else {  /* the idx is beyond the number of sections */
            *len = 0;
            return NULL;
        }
    }
    return pre;
}

char *
get_absolute_path(const char *path, _request_type req_type)
{
    char *abs_path;
    const char *dir;

    switch (req_type) {
        case REQ_CGI: dir = g_dir_cgi; path += strlen("/cgi-bin"); break;
        case REQ_STATIC: dir = g_dir; break;
        default: return NULL;
    }

    abs_path = (char *)malloc(sizeof(char) * PATH_MAX);
    if (abs_path == NULL)
        return abs_path;

    strncpy(abs_path, dir, PATH_MAX);
    abs_path[strlen(dir)] = '\0';
    strncat(abs_path, path, PATH_MAX - strlen(dir));
    abs_path[PATH_MAX - 1] = '\0';

    return realpath(abs_path, NULL);
}
