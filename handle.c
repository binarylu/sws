#include "handle.h"
#include "net.h"

static _request_type get_request_type(_request *request);

int
handle(_connection *connection)
{
    char ip[INET6_ADDRSTRLEN];
    char request_time[64];
    const char *request_line = NULL;
    size_t request_line_len;
    int nread, nwrite;
    _request *request;
    _response *response;

    if (connection->buf == NULL) {
        connection->buf = (char *)malloc(sizeof(char) * BUFFSIZE);
        if (connection->buf == NULL) {
            RET_ERRORP(-2, "Fail to allocate memory for connection buffer!");
        }
        memset(connection->buf, 0, BUFFSIZE);
    }
    nread = read(connection->fd, connection->buf + connection->pos, BUFFSIZE);
    if (nread < 0) {
        if (connection->buf != NULL) {
            free(connection->buf);
            connection->buf = NULL;
        }
        WARNP("Fail to read connection buffer");
        return -1;
    } else if (nread == 0) {
        if (connection->buf != NULL) {
            free(connection->buf);
            connection->buf = NULL;
        }
        return 0;
    } else {
        connection->pos += nread;
        if (strlen(connection->buf) < 5)
            return 2;
        if (strncmp(connection->buf + strlen(connection->buf) - 4, "\r\n\r\n", 4) != 0) {
            return 2;
        } else {
            get_date_rfc1123(request_time, sizeof(request_time));
            sockaddr2string((struct sockaddr *)(connection->addr), ip);

            DEBUG("========== Request =========");
            DEBUG("Request from %s->\n%s", ip, connection->buf);

            request = connection->request;
            response = connection->response;

            if (decode_request(connection->buf, request) != 0) {
                if (connection->buf != NULL) {
                    free(connection->buf);
                    connection->buf = NULL;
                }
                RET_ERROR(-1, "Decode request error");
            }

#ifdef DEVELOPMENT
            DEBUG("========== Request decode =========");
            _header_entry *p;
            DEBUG("%d %s %s", request->method, request->uri, request->version);
            p = request->header_entry;
            while (p) {
                DEBUG("%s =====>>>>>> %s", p->key, p->value);
                p = p->next;
            }
#endif
            char *user_prefix = NULL;

            DEBUG("========== Request handle =========");
            switch (get_request_type(request)) {
                case REQ_CGI:
                    handle_cgi(request, response);
                    DEBUG("cgi abs path: %s", get_absolute_path(request->uri, REQ_CGI, &user_prefix));
                    break;
                case REQ_STATIC:
                    handle_static(request, response);
                    DEBUG("static abs path: %s", get_absolute_path(request->uri, REQ_STATIC, &user_prefix));
                    break;
                default: handle_static(request, response);
            }
            if (user_prefix != NULL) {
                free(user_prefix);
                user_prefix = NULL;
            }

            char *resp = encode_response(response);
            if (resp == NULL) {
                if (connection->buf != NULL) {
                    free(connection->buf);
                    connection->buf = NULL;
                }
                WARNP("Fail to encode response");
                return -1;
            }

            DEBUG("========== Response =========");
            DEBUG("Response to %s->\n%s", ip, response->body == NULL ? "" : response->body);
            if ((nwrite = send(connection->fd, resp, strlen(resp), 0)) < 0) {
                WARNP("Failed to send");
            }
            if (resp != NULL) {
                free(resp);
                resp = NULL;
            }

            request_line = seperate_string(connection->buf, "\r", &request_line_len, 0);
            if (request_line != NULL)
                connection->buf[request_line_len] = '\0';
            LOG("%s %s %s %u %ld", ip,
                    request_time,
                    connection->buf,
                    response->code,
                    (long int)strlen(response->body == NULL ? "" : response->body));
            DEBUG("after: %s %s %s %s %s %u %ld", ip,
                    request_time,
                    request->method == GET_METHOD ? "GET" :
                    (request->method == HEAD_METHOD ? "HEAD" : "OTHER"),
                    request->uri == NULL ? "" : request->uri,
                    request->version == NULL ? "" : request->version,
                    response->code,
                    strlen(response->body == NULL ? "" : response->body));

            free(connection->buf);
        }
    }
    return 1;
}

static _request_type
get_request_type(_request *request)
{
    if (request->uri == NULL)
        return REQ_STATIC;
    if (g_dir_cgi != NULL && strncmp(request->uri, "/cgi-bin", 8) == 0)
        return REQ_CGI;
    return REQ_STATIC;
}
