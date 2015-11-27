#include "net.h"

static int
init_net(const char *address, const char *port, int **listen_sock)
{
    int s, v, i;
    int *socks, sock;
    int sock_num = 0;
    struct addrinfo hints, *result = NULL, *rp = NULL;

#if 1
    char ip[INET6_ADDRSTRLEN] = {0};
    unsigned short int p;
#endif

    memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = IPPROTO_IP;
	/*hints.ai_canonname = NULL;
	hints.ai_addr = (struct sockaddr *)&server;
	hints.ai_next = NULL;*/

    if ((s = getaddrinfo(address, port, &hints, &result)) != 0) {
        fprintf(stderr, "Invalid address: %s.(%s)\n", address, gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    rp = result;
    while (rp) {
        ++sock_num;
        rp = rp->ai_next;
    }
#if 1
    printf("sock_num: %d\n", sock_num);
#endif
    *listen_sock = (int *)malloc(sizeof(int) * sock_num);
    assert(*listen_sock);

    socks = *listen_sock;

    for (rp = result, i = 0; rp != NULL; rp = rp->ai_next) {
        if (rp->ai_family != AF_INET && rp->ai_family != AF_INET6) {
            --sock_num;
            continue;
        }
#if 1
        sockaddr2string((struct sockaddr *)rp->ai_addr, ip);
        p = get_port((struct sockaddr *)rp->ai_addr);
		if (rp->ai_family == AF_INET)
			printf("Listen on: ipv4: %s, port: %d\n", ip, p);
        else if (rp->ai_family == AF_INET6)
			printf("Listen on: ipv6: %s, port: %d\n", ip, p);
        else
            printf("Listen on: unknown family: %d\n", rp->ai_family);
#endif
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1) {
            perror("socket");
            --sock_num;
            continue;
        }
        v = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(v)) < 0) {
            perror("setsockopt");
            --sock_num;
            close(sock);
            continue;
        }
        if (bind(sock, rp->ai_addr, rp->ai_addrlen) < 0) {
            perror("bind");
            --sock_num;
            close(sock);
            continue;
        }
        if (listen(sock, MAX_LISTEN) < 0) {
            perror("listen");
            --sock_num;
            close(sock);
            continue;
        }
        socks[i++] = sock;
    }
    freeaddrinfo(result);

    return sock_num;
}

void
network_loop(char *address, char *port)
{
    int *listen_sock = NULL;
    int sock_num;
    int connfd;

    struct sockaddr_storage client_addr;
    socklen_t client_addrlen;

    int fd_max = -1, maxi = -1;
    fd_set fdset, rset;
    _connection connection[FD_SETSIZE];

    int i, m, handle_ret;

#if 1
    char ip[INET6_ADDRSTRLEN] = {0};
#endif

    if ((sock_num = init_net(address, port, &listen_sock)) == 0) {
        fprintf(stderr, "Fail to initial network!\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < FD_SETSIZE; ++i) {
        connection[i].fd = -1;
        connection[i].addr = (struct sockaddr_storage *)malloc(sizeof(struct sockaddr_storage));
        assert(connection[i].addr);
        connection[i].request = (_request *)malloc(sizeof(_request));
        assert(connection[i].request);
        connection[i].response = (_response *)malloc(sizeof(_response));
        assert(connection[i].response);
    }

    FD_ZERO(&fdset);
    for (i = 0; i < sock_num; ++i) {
        if (fd_max < listen_sock[i])
            fd_max = listen_sock[i];
        FD_SET(listen_sock[i], &fdset);
    }

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
        for (m = 0; m < sock_num; ++m) {
            if (FD_ISSET(listen_sock[m], &rset)) {
                client_addrlen = sizeof(client_addr);
                memset(&client_addr, 0, client_addrlen);
                if ((connfd = accept(listen_sock[m], (struct sockaddr *)&client_addr, &client_addrlen)) <= 0) {
                    perror("accept");
                } else {
                    for (i = 0; i < FD_SETSIZE; ++i) {
                        if (connection[i].fd < 0) {
                            SET_CONNECTION(connection[i], connfd, client_addr);
                            sockaddr2string((struct sockaddr *)(connection[i].addr), ip);
                            printf("Get connection from %s.\n", ip);
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
            if (connection[i].fd < 0)
                continue;
            if (FD_ISSET(connection[i].fd, &rset)) {
                handle_ret = handle(&(connection[i]));
                if (handle_ret < 0) {
                    perror("read");
                    close(connection[i].fd);
                    RESET_CONNECTION(connection[i]);
                } else if (handle_ret == 0) {
                    sockaddr2string((struct sockaddr *)(connection[i].addr), ip);
                    fprintf(stdout, "Connection with client %s is closed\n", ip);
                    FD_CLR(connection[i].fd, &fdset);
                    close(connection[i].fd);
                    RESET_CONNECTION(connection[i]);
                } else {
                    FD_CLR(connection[i].fd, &fdset);
                    fprintf(stdout, "Close the connection with client %s\n", ip);
                    close(connection[i].fd);
                    RESET_CONNECTION(connection[i]);
                }
            }
        }
    }

    for (i = 0; i < sock_num; ++i)
        close(listen_sock[i]);
    for (i = 0; i < FD_SETSIZE; ++i) {
        free(connection[i].request);
        free(connection[i].response);
        free(connection[i].addr);
    }

    free(listen_sock);

    destroy_handle();
}

int
sockaddr2string(struct sockaddr *sa, char *address)
{
    struct sockaddr_in *server;
    struct sockaddr_in6 *server6;
    if (sa->sa_family == AF_INET) {
        server = (struct sockaddr_in *)sa;
        if (inet_ntop(sa->sa_family, &(server->sin_addr), address, INET_ADDRSTRLEN) == NULL) {
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

unsigned short int
get_port(struct sockaddr *sa)
{
    struct sockaddr_in *server;
    struct sockaddr_in6 *server6;
    if (sa->sa_family == AF_INET) {
        server = (struct sockaddr_in *)sa;
        return ntohs(server->sin_port);
    } else if (sa->sa_family == AF_INET6) {
        server6 = (struct sockaddr_in6 *)sa;
        return ntohs(server6->sin6_port);
    } else {
        printf("family: %d\n", sa->sa_family);
        return -1;
    }
    return -1;
}

/*
 * (deprecated) Use getaddrinfo to verify
 */
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
validate_port(char *str)
{
    int i;
    int len = strlen(str);
    for (i = 0; i < len; ++i)
        if (str[i] < '0' || str[i] > '9')
            return 0;
    if (atoi(str) > 65535)
        return 0;
    return 1;
}
