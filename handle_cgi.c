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
    char time_buff[MAX_TIME_SIZE];
    int status = 0;

    get_date_rfc1123(time_buff, MAX_TIME_SIZE);

    if (response_addfield(resp, "Date", 4, time_buff, strlen(time_buff)) != 0) {
        resp->code = 500;
        generate_desc(resp);
        handleError(resp);
        return 0;
    }

    /* no file existed, respond 404 */
    if (stat(cgipath, &pathstat) == -1) {
        DEBUGP("Fail to stat file");
        resp->code = 404;
        generate_desc(resp);
        handleError(resp);
        return 0;
    }

    /* not regular file, respond 403 */
    if (!S_ISREG(pathstat.st_mode)) {
        DEBUGP("Not regular file");
        resp->code = 403;
        generate_desc(resp);
        handleError(resp);
        return 0;
    }

    /* can't execute file, respond 403 */
    if (access(cgipath, X_OK) == -1) {
        DEBUGP("Fail to access file");
        resp->code = 403;
        generate_desc(resp);
        handleError(resp);
        return 0;
    }

    if (pipe(pipefd) == -1) {
        WARNP("Fail to create pipe");
        resp->code = 500;
        generate_desc(resp);
        handleError(resp);
        return 0;
    }

    pid = fork();
    if (pid == -1) {
        WARNP("Fail to fork");
        resp->code = 500;
        generate_desc(resp);
        handleError(resp);
        return 0;
    } else if (pid == 0) {
        while ((dup2(pipefd[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
        close(pipefd[0]);
        if (execl(cgipath, "", NULL) == -1) {
            WARNP("Fail to execl");
            exit(1);
        }
        exit(0);
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
                    WARNP("Fail to read pipe");
                    resp->code = 500;
                    generate_desc(resp);
                    handleError(resp);
                    return 0;
                }
            } else if (count == 0) {
                break;
            }
        }

        close(pipefd[0]);
        if (wait(&status) == -1) {
            WARNP("Fail to wait");
        }

        if (WIFEXITED(status) == 0 ||  WEXITSTATUS(status) != 0) {
            WARN("CGI exit with error");
            resp->code = 500;
            generate_desc(resp);
            handleError(resp);
            return 0;
        }

        cgi_respond_ok(resp, buffer, strlen(buffer));
    }
    return 1;
}

void
cgi_respond_ok(_response *resp, char *buffer, int buflen)
{
    resp->code = 200;
    generate_desc(resp);
    resp->body = buffer;
    resp->is_cgi = 1;
}
