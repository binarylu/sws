#include "handle.h"
#include "net.h"

int
handle(_client_info *client)
{
    char buf[BUFFSIZE];
    struct sockaddr_storage *client_addr;
    static char ip[INET6_ADDRSTRLEN];
    int fd, nread;

    fd = client->fd;
    client_addr = &(client->addr);
    sockaddr2string((struct sockaddr *)&client_addr, ip);

    memset(buf, 0, BUFFSIZE);
    nread = read(fd, buf, BUFFSIZE);
    if (nread < 0) {
        return -1;
    } else if (nread == 0) {
        return 0;
    } else {
        fprintf(stdout, "Client %s says: %s\n", ip, buf);
    }
    return 1;
}
