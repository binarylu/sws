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
        /*fprintf(stdout, "**%d(%ld)**Client %s says: %s\n", ++i, strlen(buf), ip, buf);*/
        client->pos += nread;
        if (strncmp(client->buf + strlen(client->buf) - 2, "\r\n", 2) == 0) {
            fprintf(stdout, "**%ld**Client %s says: %s\n", strlen(client->buf), ip, client->buf);
        }
    }
    return 1;
}
