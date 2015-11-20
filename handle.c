#include "handle.h"
#include "net.h"

static _request_type get_request_type(_request *request);

int
init_handle()
{
    return 0;
}

void
destroy_handle()
{
}

int
handle(_connection *connection)
{
    struct sockaddr_storage *client_addr;
    char ip[INET6_ADDRSTRLEN];
    char request_time[64];
    int nread;
    _request *request;
    _response *response;

    _header_entry *p;

    if (connection->buf == NULL) {
        connection->buf = (char *)malloc(sizeof(char) * BUFFSIZE);
        if (connection->buf == NULL) {
            perror("Fail to allocate memory for connection buffer!");
            return -2;
        }
        memset(connection->buf, 0, BUFFSIZE);
    }
    nread = read(connection->fd, connection->buf + connection->pos, BUFFSIZE);
    if (nread < 0)
        return -1;
    else if (nread == 0)
        return 0;
    else {
        connection->pos += nread;
        if (strncmp(connection->buf + strlen(connection->buf) - 2, "\r\n", 2) == 0) {
            get_date_rfc1123(request_time, sizeof(request_time));
            client_addr = &(connection->addr);
            sockaddr2string((struct sockaddr *)client_addr, ip);

            printf("Request from %s->\n%s\n", ip, connection->buf);
            printf("=============================\n\n");

            request_init(request = &(connection->request));
            response_init(response = &(connection->response));

            if (decode_request(connection->buf, request) != 0) {
                perror("decode request error");
                return -1;
            }

            printf("%d %s %s\n", request->method, request->uri, request->version);
            p = request->header_entry;
            while (p) {
                printf("%s =====>>>>>> %s\n", p->key, p->value);
                p = p->next;
            }

            switch (get_request_type(request)) {
                case REQ_CGI:
                    handle_cgi(request, response);
                    printf("cgi abs path: %s\n", get_absolute_path(request->uri, REQ_CGI));
                    break;
                case REQ_STATIC:
                    handle_cgi(request, response);
                    printf("static abs path: %s\n", get_absolute_path(request->uri, REQ_STATIC));
                    break;
                case REQ_OTHER: handle_other(request, response); break;
                default: handle_other(request, response);
            }

            encode_response(response, connection->buf);

            printf("=============================\n\n");
            printf("Response to %s->\n%s\n", ip, connection->buf);

            LOG("%s %s %s %s %s %u %ld\n", ip,
                    request_time,
                    request->method == GET_METHOD ? "GET" :
                    (request->method == HEAD_METHOD ? "HEAD" : "NONE"),
                    request->uri, request->version,
                    response->code, strlen(response->body));

            request_clear(request);
            response_clear(response);
            free(connection->buf);
        }
    }
    return 1;
}

/* check the path security */
static _request_type
get_request_type(_request *request)
{
    if (g_dir_cgi != NULL && strncmp(request->uri, "/cgi-bin", 8) == 0)
        return REQ_CGI;
    return REQ_STATIC;
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

int
validate_path(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

int
validate_path_security(const char *path, _request_type req_type)
{
    int ret = 0;
    char *real_path;
    const char *server_dir;
    char *real_server_dir;
    size_t len_path, len_dir;

    switch (req_type) {
        case REQ_CGI: server_dir = g_dir_cgi; break;
        case REQ_STATIC: server_dir = g_dir; break;
        default: return 0;
    }
    if (server_dir == NULL)
        return 0;

    do {
        if((real_path = realpath(path, NULL)) == NULL ||
                (real_server_dir = realpath(g_dir, NULL)) == NULL)
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
