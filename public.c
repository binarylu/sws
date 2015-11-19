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

    assert(strncpy(node->key, key, keylen + 1));
    assert(strncpy(node->value, val, vallen + 1));

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
    if ((g_logfd = open(filename, O_WRONLY | O_APPEND | O_CREAT)) == -1) {
        perror("Fail to open log file");
        return -1;
    }
    return 0;
}

void
close_log()
{
    close(g_logfd);
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

int
validate_ipv4(const char *ip)
{
    int i, cnt = 0, sum;
    const char *p;
    size_t len;
    while ((p = seperate_string(ip, ".", &len, cnt++)) != NULL) {
        sum = 0;
        for (i = 0; i < len; ++i) {
            if (!isdigit(p[i]))
                return 0;
            sum = sum * 10 + p[i] - '0';
        }
        if (sum > 255)
            return 0;
    }
    if (cnt > 5) /* The fifth try can know it is the end */
        return 0;
    return 1;
}
