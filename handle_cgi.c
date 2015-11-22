#include "handle_cgi.h"

void cgi_respond_404(_response *);
int cgi_respond(_response *, char *, int);

int
handle_cgi(/*Input*/const _request *req, /*Output*/_response *resp)
{
    char *cgipath = get_absolute_path(req->uri, REQ_CGI);

    if (access(cgipath, X_OK) == -1) {
        perror("access");
        cgi_respond_404(resp);
        exit(1);
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        while ((dup2(pipefd[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
        close(pipefd[0]);
        execl(req->uri, (char*)0);
        perror("execl");
        _exit(1);
    } else {
        close(pipefd[1]);
        char buffer[4096];
        while (1) {
            ssize_t count = read(pipefd[0], buffer, sizeof(buffer));
            if (count == -1) {
                if (errno == EINTR) {
                    continue;
                } else {
                    perror("read");
                    exit(1);
                }
            } else if (count == 0) {
                break;
            } else {
                cgi_respond(resp, buffer, count);
            }
        }
        close(pipefd[0]);
        wait(0);
    }
    return 0;
}

void
cgi_respond_404(_response *resp)
{
    resp->code = 404;
    resp->desc = "Not Found";
}

int
cgi_respond(_response *resp, char *buffer, int count)
{
    return 0;
}


