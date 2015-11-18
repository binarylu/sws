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
handle(_client_info *client)
{
    struct sockaddr_storage *client_addr;
    char ip[INET6_ADDRSTRLEN];
    int fd, nread;
    _request request;
    _response response;
    _header *p;

    fd = client->fd;
    client_addr = &(client->addr);
    sockaddr2string((struct sockaddr *)client_addr, ip);

    if (client->buf == NULL) {
        client->buf = (char *)malloc(sizeof(char) * BUFFSIZE);
        memset(client->buf, 0, BUFFSIZE);
    }
    nread = read(fd, client->buf + client->pos, BUFFSIZE);
    if (nread < 0)
        return -1;
    else if (nread == 0)
        return 0;
    else {
        client->pos += nread;
        if (strncmp(client->buf + strlen(client->buf) - 2, "\r\n", 2) == 0) {
            fprintf(stdout, "**%ld**Client %s says: %s\n", strlen(client->buf), ip, client->buf);
            printf("=============================\n\n");
            decode_request(client->buf, &request);
            printf("%d %s %s\n", request.method, request.uri, request.version);
            p = request.head_entry;

            while (p) {
                printf("%s => %s\n", p->key, p->value);
                p = p->next;
            }
            /*encode_response(&response, client->buf);*/
            free(client->buf);
        }
    }
    return 1;
}
