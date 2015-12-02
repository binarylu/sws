#include "public.h"

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
    req->errcode = NO_ERR;
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

    assert(strncpy(node->key, key, keylen));
    assert(strncpy(node->value, val, vallen));
    node->key[keylen] = '\0';
    node->value[vallen] = '\0';
    node->next = NULL;

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
    resp->version = "HTTP/1.0";
    resp->header_entry = NULL;
    resp->body = NULL;
    resp->is_cgi = 0;
    response_addfield(resp, "Server", 6, "sws 1.0", 7);
}

void
response_clear(_response *resp)
{
    if (resp->desc != NULL)
        free(resp->desc);
    /*if (resp->version != NULL)
        free(resp->version);*/
    if (resp->body != NULL)
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

    assert(strncpy(node->key, key, keylen));
    assert(strncpy(node->value, val, vallen));
    node->key[keylen] = '\0';
    node->value[vallen] = '\0';
    node->next = NULL;

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
    FILE *f;
    /*char t[64];*/
    if ((f = fopen(filename, "a")) == NULL) {
        return -1;
    }
    /*get_date_rfc1123(t, sizeof(t));
    fprintf(f, "%s: sws starts!\n", t);*/
    /*setbuf(g_log, NULL);*/
    fclose(f);
    return 0;
}
/*
void
close_log()
{
    fclose(g_log);
}*/

void
get_year_mon_day(int* year, int* mon, int* day)
{
    time_t timep;
    struct tm *p;

    time(&timep);
    p = gmtime(&timep);
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
    p = gmtime(&timep);
    strftime(buf, len, "%a, %d %b %Y %T GMT", p);
}

void
get_date_rfc850(char *buf, size_t len)
{
    time_t timep;
    struct tm *p;

    time(&timep);
    p = gmtime(&timep);
    strftime(buf, len, "%A, %d-%b-%y %T GMT", p);
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
validate_path(const char *path)
{
    struct stat st;
    if (path == NULL)
        return 0;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

int
validate_path_security(const char *path, _request_type req_type)
{
    int ret = 0;
    const char *server_dir = NULL;
    char *real_path = NULL;
    char *real_server_dir = NULL;
    size_t len_path, len_dir;

    switch (req_type) {
        case REQ_CGI: server_dir = g_dir_cgi; break;
        case REQ_STATIC: server_dir = g_dir; break;
        default: return 0;
    }
    if (server_dir == NULL)
        return 0;

    do {
        real_path = realpath(path, NULL);
        if (real_path == NULL)
            break;
        real_server_dir = realpath(server_dir, NULL);
        if (real_server_dir == NULL)
            break;

        /*printf("path: %s\ndir: %s\n", real_path, real_server_dir);*/

        len_path = strlen(real_path);
        len_dir = strlen(real_server_dir);

        if (len_path < len_dir ||
                strncmp(real_path, real_server_dir, len_dir) != 0)
            break;

        ret = 1;
    } while(0);

    if (real_path)
        free(real_path);
    if (real_server_dir)
        free(real_server_dir);
    return ret;
}

char *
get_absolute_path(const char *path, _request_type req_type)
{
    char *abs_path = NULL;
    const char *username = NULL;
    size_t username_len;

    abs_path = (char *)malloc(sizeof(char) * PATH_MAX);
    if (abs_path == NULL)
        return NULL;

    if (req_type == REQ_CGI) {
        path += strlen("/cgi-bin");
        if (g_dir_cgi[strlen(g_dir_cgi)-1] == '/' && path[0] == '/')
            snprintf(abs_path, PATH_MAX, "%s%s", g_dir_cgi, path+1);
        else
            snprintf(abs_path, PATH_MAX, "%s%s", g_dir_cgi, path);

    } else if (req_type == REQ_STATIC) {
        username = seperate_string(path, "/", &username_len, 1);
        --username_len;
        if (username != NULL && username[0] == '~') {
            path += username_len + 2;
            snprintf(abs_path, 6 + username_len + 1, "/home/%s", username + 1);
            snprintf(abs_path + 6 + username_len,
                    PATH_MAX - (6 + username_len), "/sws%s", path);
        } else {
            if (g_dir[strlen(g_dir)-1] == '/' && path[0] == '/')
                snprintf(abs_path, PATH_MAX, "%s%s", g_dir, path+1);
            else
                snprintf(abs_path, PATH_MAX, "%s%s", g_dir, path);
        }
    } else {
        return NULL;
    }

    return abs_path;
}
