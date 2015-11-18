#include "handle.h"
#include "net.h"

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
    int fd, nread;
    _request request;
    _response response;

    _header_entry *p;

    fd = connection->fd;
    client_addr = &(connection->addr);
    sockaddr2string((struct sockaddr *)client_addr, ip);

    if (connection->buf == NULL) {
        connection->buf = (char *)malloc(sizeof(char) * BUFFSIZE);
        memset(connection->buf, 0, BUFFSIZE);
    }
    nread = read(fd, connection->buf + connection->pos, BUFFSIZE);
    if (nread < 0)
        return -1;
    else if (nread == 0)
        return 0;
    else {
        connection->pos += nread;
        if (strncmp(connection->buf + strlen(connection->buf) - 2, "\r\n", 2) == 0) {
            printf("Request from %s->\n%s\n", ip, connection->buf);
            printf("=============================\n\n");

            request_init(&(connection->request));
            response_init(&(connection->response));

            decode_request(connection->buf, &request);
            printf("%d %s %s\n", request.method, request.uri, request.version);
            p = (connection->request).header_entry;
            while (p) {
                printf("%s => %s\n", p->key, p->value);
                p = p->next;
            }


            handle_static(&(connection->request), &(connection->response));
            handle_cgi(&(connection->request), &(connection->response));
            handle_other(&(connection->request), &(connection->response));

            encode_response(&response, connection->buf);

            request_clear(&(connection->request));
            response_clear(&(connection->response));

            printf("=============================\n\n");
            printf("Response to %s->\n%s\n", ip, connection->buf);

            free(connection->buf);
        }
    }
    return 1;
}
