#include "handle_cgi.h"
#include "handle_response.h"

void cgi_respond_ok(_response *, char *, int);

int
handle_cgi(/*Input*/const _request *req, /*Output*/_response *resp)
{
    char *cgipath = get_absolute_path(req->uri, REQ_CGI);
    struct stat pathstat;
    int pipefd[2];
    pid_t pid;

    /* no file existed, respond 404 */
    if(stat(cgipath, &pathstat) == -1) {
        perror("stat");
        respond_not_found(resp);
        return 0;
    }

    /* not regular file, respond 403 */
    if(!S_ISREG(pathstat.st_mode)) {
        respond_forbidden(resp);
        return 0;
    }

    /* can't execute file, respond 403 */
    if (access(cgipath, X_OK) == -1) {
        perror("access");
        respond_forbidden(resp);
        return 0;
    }

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        while ((dup2(pipefd[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
        close(pipefd[0]);
        execl(cgipath, "", NULL);
        perror("execl");
        _exit(1);
    } else {
        close(pipefd[1]);
        char *buffer = (char *)malloc(4096);
        size_t pos = 0;
        while (1) {
            ssize_t count = read(pipefd[0], buffer + pos, sizeof(buffer));
            pos += count;
            if (count == -1) {
                if (errno == EINTR) {
                    continue;
                } else {
                    perror("read");
                    exit(1);
                }
            } else if (count == 0) {
                break;
            }
        }
	    cgi_respond_ok(resp, buffer, strlen(buffer));
        close(pipefd[0]);
        wait(0);
    }
    return 0;
}

void
cgi_respond_ok(_response *resp, char *buffer, int buflen)
{
    resp->code = 200;
    resp->desc = generate_str("OK");
    resp->body = buffer;
    resp->is_cgi = 1;
}
