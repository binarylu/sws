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
    char ip[INET6_ADDRSTRLEN];
    char request_time[64];
    int nread, nwrite;
    _request *request;
    _response *response;

#if DEVELOPMENT
    _header_entry *p;
#endif

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
            sockaddr2string((struct sockaddr *)(connection->addr), ip);

#if DEVELOPMENT
            printf("Request from %s->\n%s\n", ip, connection->buf);
            printf("=============================\n\n");
#endif

            request = connection->request;
            response = connection->response;
            request_init(request);
            response_init(response);

            if (decode_request(connection->buf, request) != 0) {
                perror("decode request error");
                return -1;
            }

#if DEVELOPMENT
            printf("%d %s %s\n", request->method, request->uri, request->version);
            p = request->header_entry;
            while (p) {
                printf("%s =====>>>>>> %s\n", p->key, p->value);
                p = p->next;
            }
#endif

            switch (get_request_type(request)) {
                case REQ_CGI:
                    handle_cgi(request, response);
#if DEVELOPMENT
                    printf("cgi abs path: %s\n", get_absolute_path(request->uri, REQ_CGI));
#endif
                    break;
                case REQ_STATIC:
                    handle_static(request, response);
#if DEVELOPMENT
                    printf("static abs path: %s\n", get_absolute_path(request->uri, REQ_STATIC));
#endif
                    break;
                case REQ_OTHER: handle_other(request, response); break;
                default: handle_other(request, response);
            }

            char *resp = encode_response(response);

#if DEVELOPMENT
            printf("=============================\n\n");
            printf("Response to %s->\n%s\n", ip, response->body);
#endif
            if ((nwrite = send(connection->fd, resp, strlen(resp), 0)) < 0) {
                fprintf(stderr, "send failed\n");
            }

            LOG("%s %s %s %s %s %u %ld\n", ip,
                    request_time,
                    request->method == GET_METHOD ? "GET" :
                    (request->method == HEAD_METHOD ? "HEAD" : "NONE"),
                    request->uri, request->version,
                    response->code,
                    strlen(response->body == NULL ? "" : response->body));

            request_clear(request);
            response_clear(response);
            free(connection->buf);
        }
    }
    return 1;
}

static _request_type
get_request_type(_request *request)
{
    if (g_dir_cgi != NULL && strncmp(request->uri, "/cgi-bin", 8) == 0)
        return REQ_CGI;
    return REQ_STATIC;
}
