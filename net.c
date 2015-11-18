#include "net.h"

#define SET_CLIENT(c, _fd, _addr) do { \
    (c).fd = (_fd); \
    (c).buf = NULL; \
    (c).pos = 0; \
    (c).addr = (_addr); \
} while(0)

#define RESET_CLIENT(c) do { \
    (c).fd = -1; \
    (c).buf = NULL; \
    (c).pos = 0; \
} while(0)

static int init_net(char *address, char *port, int *sock);

int
init_net(char *address, char *port, int *sock)
{
    int s, v, i;
    struct addrinfo hints, *result, *rp;

    memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;
	hints.ai_protocol = IPPROTO_IP;
	/*hints.ai_canonname = NULL;
	hints.ai_addr = (struct sockaddr *)&server;
	hints.ai_next = NULL;*/

    if ((s = getaddrinfo(address, port, &hints, &result)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    for (rp = result, i = 0; rp != NULL && i < 2; rp = rp->ai_next, ++i) {
        sock[i] = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock[i] == -1) {
            perror("socket");
            continue;
        }
        v = 1;
        if (setsockopt(sock[i], SOL_SOCKET, SO_REUSEADDR, &v, sizeof(v)) < 0) {
            perror("setsockopt");
            return -1;
        }
        if (bind(sock[i], rp->ai_addr, rp->ai_addrlen) < 0) {
            perror("socket");
            return -1;
        }
        if (listen(sock[i], MAX_LISTEN) < 0) {
            perror("listen");
            return -1;
        }
    }
    freeaddrinfo(result);

    return 0;
}

int
sockaddr2string(struct sockaddr *sa, char *address)
{
    struct sockaddr_in *server;
    struct sockaddr_in6 *server6;
    if (sa->sa_family == AF_INET) {
        server = (struct sockaddr_in *)sa;
        if (inet_ntop(sa->sa_family, &(server->sin_addr), address, INET6_ADDRSTRLEN) == NULL) {
            perror("inetV4_ntop");
            return -1;
        }
    } else if (sa->sa_family == AF_INET6) {
        server6 = (struct sockaddr_in6 *)sa;
        if (inet_ntop(sa->sa_family, &(server6->sin6_addr), address, INET6_ADDRSTRLEN) == NULL) {
            perror("inetV6_ntop");
            return -1;
        }
    } else {
        printf("family: %d\n", sa->sa_family);
        return -1;
    }
    return 0;
}

void
network_loop(char *address, char *port)
{
    int listen_sock[2], connfd, fd_max = -1;
    struct sockaddr_storage client_addr;
    socklen_t client_addrlen;
    fd_set fdset, rset;
    _client_info client[FD_SETSIZE];
    char ip[INET6_ADDRSTRLEN] = {0};
    int i, m, maxi = -1, handle_ret;

    client_addrlen = sizeof(struct sockaddr_in);

    if (init_net(address, port, listen_sock) < 0) {
        fprintf(stderr, "Fail to initial network!\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < FD_SETSIZE; ++i)
        client[i].fd = -1;

    fd_max = listen_sock[0] > listen_sock[1] ? listen_sock[0] : listen_sock[1];
    FD_ZERO(&fdset);
    FD_SET(listen_sock[0], &fdset);
    FD_SET(listen_sock[1], &fdset);
    if (init_handle() != 0) {
        fprintf(stderr, "Fail to initialize handle!");
        exit(EXIT_FAILURE);
    }
    for (;;) {
        rset = fdset;
        if (select(fd_max + 1, &rset, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }
        for (m = 0; m < 2; ++m) {
            if (FD_ISSET(listen_sock[m], &rset)) {
                client_addrlen = sizeof(client_addr);
                memset(&client_addr, 0, client_addrlen);
                if ((connfd = accept(listen_sock[m], (struct sockaddr *)&client_addr, &client_addrlen)) <= 0) {
                    perror("accept");
                } else {
                    for (i = 0; i < FD_SETSIZE; ++i) {
                        if (client[i].fd < 0) {
                            SET_CLIENT(client[i], connfd, client_addr);
                            /*sockaddr2string((struct sockaddr *)&client_addr, ip);
                            printf("Get connection from %s.\n", ip);*/
                            break;
                        }
                    }
                    if (i == FD_SETSIZE)
                        fprintf(stderr, "Too many connections.");
                    FD_SET(connfd, &fdset);
                    if (connfd > fd_max)
                        fd_max = connfd;
                    if (i > maxi)
                        maxi = i;
                }
            }
        }
        for (i = 0; i <= maxi; ++i) {
            if (client[i].fd < 0)
                continue;
            if (FD_ISSET(client[i].fd, &rset)) {
                handle_ret = handle(&(client[i]));
                if (handle_ret == -1) {
                    perror("read");
                    close(client[i].fd);
                    RESET_CLIENT(client[i]);
                } else if (handle_ret == 0) {
                    sockaddr2string((struct sockaddr *)&client_addr, ip);
                    fprintf(stdout, "Connection with client %s is closed\n", ip);
                    FD_CLR(client[i].fd, &fdset);
                    close(client[i].fd);
                    RESET_CLIENT(client[i]);
                }
            }
        }
    }
    close(listen_sock[0]);
    close(listen_sock[1]);
}
