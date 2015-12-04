#include "handle_static.h"

    int
handle_static(/*Input*/const _request *request, /*Output*/_response *response)
{
    char time_buff[MAX_TIME_SIZE];
    char *path;
    char *user_prefix = NULL;
    int http_code;
    struct stat req_stat;

    get_date_rfc1123(time_buff, MAX_TIME_SIZE);
    if (response_addfield(response, "Date", 4, time_buff, strlen(time_buff)) != 0) {
        response->code = 500;
        generate_desc(response);
        handleError(response);
        return 0;
    }
    if ((http_code = validate_request(request, response)) != 0) {
        generate_response(http_code, response);
        return 0;
    }

    path = get_absolute_path(request->uri, REQ_STATIC, &user_prefix);
    if (path == NULL) {
        generate_response(500, response);
        return 0;
    }

    do {
        if ((http_code = validate_stat(path, response, &req_stat)) != 0) {
            generate_response(http_code, response);
            break;
        }

        if (validate_path_security(path, REQ_STATIC, user_prefix) == 0){
            generate_response(403, response);
            break;
        }

        if (S_ISREG(req_stat.st_mode)) {                         /* For regular file, respond the its content directly*/
            if (set_file(request, path, &req_stat, response) != 0) {
                generate_response(500, response);
                break;
            }
        } else if (S_ISDIR(req_stat.st_mode)) {                  /* For directory*/
            strncat(path, "/", PATH_MAX - strlen(path) - 1);
            strncat(path, INDEX, PATH_MAX - strlen(path) - 1);
            http_code = validate_stat(path, response, &req_stat);
            if (http_code == 0) {                                /* index.html file exists*/
                if (set_file(request, path, &req_stat, response) != 0) {
                    generate_response(500, response);
                    break;
                }
            } else if (http_code == 404) {                       /* index.html file does not exist*/
                path[strlen(path) - strlen(INDEX) - 1] = '\0';
                if (set_directory(request, path, &req_stat, response) != 0) {
                    generate_response(500, response);
                    break;
                }
            } else {                                            /* index.html has a permission issue*/
                generate_response(http_code, response);
                break;
            }
        }

    } while ( /* CONSTCOND */ 0 );

    free(path);

    return 0;
}

int
if_modified(const _request *request, const struct stat* req_stat)
{
    _header_entry* head = request->header_entry;
    for (;head;head = head->next){
        if (!strcmp(head->key, "If-Modified-Since")){
            if (same_time(head->value, req_stat->st_mtime))
                return 1;
            else return 0;
        }
    }
    return 0;
}

int
same_time(const char* val, const time_t mtime)
{
    char time_buff[MAX_TIME_SIZE];
    struct tm *p;
    p = gmtime(&mtime);

    strftime(time_buff, MAX_TIME_SIZE, "%a, %d %b %Y %T GMT", p);
    if (strcmp(val, time_buff) == 0) return 1;

    strftime(time_buff, MAX_TIME_SIZE, "%A, %d-%b-%y %T GMT", p);
    if (strcmp(val, time_buff) == 0) return 1;

    strftime(time_buff, MAX_TIME_SIZE, "%c", p);
    if (strcmp(val, time_buff) == 0) return 1;

    return 0;
}

int
set_file(const _request *request, const char *path, const struct stat *req_stat, _response *response)
{
    char time_buff[MAX_TIME_SIZE];
    struct tm *p;
    int req_fd;
    int nums;
    char str[20];
    char* mime = NULL;
    char buf[BUFF_SIZE];
    int body_size;

    if (if_modified(request, req_stat)) {
        generate_response(304, response);
        return 0;
    }

    p = gmtime(&(req_stat->st_mtime));
    strftime(time_buff, MAX_TIME_SIZE, "%a, %d %b %Y %T GMT", p);
    if (response_addfield(response, "Last-Modified", 13, time_buff, strlen(time_buff)) != 0) {
        return -1;
    }

    mime = getMIME(path);
    if (response_addfield(response, "Content-Type", 12, mime, strlen(mime)) != 0) {
        free(mime);
        return -1;
    }
    free(mime);
    snprintf(str, 20, "%ld", (long int)(req_stat->st_size));
    if (response_addfield(response, "Content-Length", 14, str, strlen(str)) != 0)
        return -1;

    if (request->method == HEAD_METHOD){
        generate_response(200, response);
        return 0;
    }

    body_size = req_stat->st_size;
    if ((response->body = (char *)malloc(body_size+1)) == NULL)
        return -1;
    (response->body)[0] = '\0';

    if ((req_fd = open(path, O_RDONLY)) == -1 )
        return -1;

    while ((nums = read(req_fd, buf, BUFF_SIZE)) > 0)
        strncat(response->body, buf, nums);

    if (nums < 0)
        return -1;

    if (close(req_fd) == -1 )
        return -1;

    generate_response(200, response);

    return 0;
}


int
set_directory(const _request *request, const char *path, struct stat *req_stat, _response *response)
{
    char str[20];
    char time_buff[MAX_TIME_SIZE];
    struct tm *p;

    if (if_modified(request, req_stat)) {
        generate_response(304, response);
        return 0;
    }

    p = gmtime(&(req_stat->st_mtime));
    strftime(time_buff, MAX_TIME_SIZE, "%a, %d %b %Y %T GMT", p);
    if (response_addfield(response, "Last-Modified", 13, time_buff, strlen(time_buff)) != 0) {
        return -1;
    }

    if (response_addfield(response, "Content-Type", 12, "text/html", 9) != 0) {
        return -1;
    }
    if ((response->body = generate_index(path)) == NULL) {
        return -1;
    }
    snprintf(str, 20, "%ld", (long int)strlen(response->body));
    if (response_addfield(response, "Content-Length", 14, str, strlen(str)) != 0) {
        return -1;
    }

    if (request->method == HEAD_METHOD){
        free(response->body);
        response->body = NULL;
    }

    generate_response(200, response);

    return 0;
}

char*
getMIME(const char* path)
{
    const char *mime;
    char *mime_ret;
#ifdef __APPLE__
    mime_ret = generate_str("text/html");
    return mime_ret;
#else
    magic_t magic;

    magic = magic_open(MAGIC_MIME_TYPE);
    magic_load(magic, NULL);
    magic_compile(magic, NULL);
    mime = magic_file(magic, path);
    mime_ret = generate_str(mime);
    magic_close(magic);

    return mime_ret;
#endif
}
