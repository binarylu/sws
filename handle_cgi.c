#include "handle_cgi.h"
#include "handle_response.h"

void cgi_respond_ok(_response *, char *, int);

int
handle_cgi(/*Input*/const _request *request, /*Output*/_response *response)
{
    char time_buff[MAX_TIME_SIZE];
    char *path;
    char *user_prefix = NULL;
    int http_code;
    struct stat req_stat;
    int pipefd[2];
    pid_t pid;
    int status = 0;

    get_date_rfc1123(time_buff, MAX_TIME_SIZE);
    if (response_addfield(response, "Date", 4, time_buff, strlen(time_buff)) != 0) {
        generate_response(500, response);
        return 0;
    }
    if ((http_code = validate_request(request, response)) != 0) {
        generate_response(http_code, response);
        return 0;
    }

    path = get_absolute_path(request->uri, REQ_CGI, &user_prefix);
    if (path == NULL) {
        generate_response(500, response);
        return 0;
    }

    do {
        if ((http_code = validate_stat(path, response, &req_stat)) != 0) {
            generate_response(http_code, response);
            break;
        }

        if (validate_path_security(path, REQ_CGI, user_prefix) == 0){
            generate_response(403, response);
            break;
        }

        /* not regular file, respond 403 */
        if (!S_ISREG(req_stat.st_mode)) {
            DEBUGP("Not regular file");
            generate_response(403, response);
            break;
        }

        /* can't execute file, respond 403 */
        if (access(path, X_OK) == -1) {
            DEBUGP("Fail to access file");
            generate_response(403, response);
            break;
        }

        if (pipe(pipefd) == -1) {
            WARNP("Fail to create pipe");
            generate_response(500, response);
            break;
        }

        pid = fork();
        if (pid == -1) {
            WARNP("Fail to fork");
            generate_response(500, response);
            break;
        } else if (pid == 0) {
            while ((dup2(pipefd[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
            close(pipefd[0]);
            if (execl(path, "", NULL) == -1) {
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
                if (count == 0)
                    break;
                if (count == -1) {
                    if (errno == EINTR)
                        continue;
                    WARNP("Fail to read pipe");
                    generate_response(500, response);
                    return 0;
                }
            }
            close(pipefd[0]);
            if (wait(&status) == -1) {
                WARNP("Fail to wait");
                generate_response(500, response);
                return 0;
            }

            if (WIFEXITED(status) == 0 ||  WEXITSTATUS(status) != 0) {
                WARN("CGI exit with error");
                generate_response(500, response);
                return 0;
            }

            buffer[pos] = '\0';
            cgi_respond_ok(response, buffer, strlen(buffer));
        }
    } while ( /* CONSTCOND */ 0 );

    free(path);

    return 0;
}

void
cgi_respond_ok(_response *resp, char *buffer, int buflen)
{
    resp->code = 200;
    generate_desc(resp);
    resp->body = buffer;
    resp->is_cgi = 1;
}