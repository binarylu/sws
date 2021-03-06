#include "handle_response.h"

void
respond_not_found(_response *resp)
{
    resp->code = 404;
    resp->desc = generate_str("Not Found");
}

void
respond_forbidden(_response *resp)
{
    resp->code = 403;
    resp->desc = generate_str("Forbidden");
}

int
generate_desc(_response* response)
{
    switch(response->code) {
        case 200:
            response->desc = generate_str("OK");
            break;
        case 201:
            response->desc = generate_str("Created");
            break;
        case 202:
            response->desc = generate_str("Accepted");
            break;
        case 204:
            response->desc = generate_str("No Content");
            break;
        case 300:
            response->desc = generate_str("Multiple Choices");
            break;
        case 301:
            response->desc = generate_str("Moved Permanently");
            break;
        case 302:
            response->desc = generate_str("Moved Temporarily");
            break;
        case 304:
            response->desc = generate_str("Not Modified");
            break;
        case 400:
            response->desc = generate_str("Bad Request");
            break;
        case 401:
            response->desc = generate_str("Unauthorized");
            break;
        case 403:
            response->desc = generate_str("Forbidden");
            break;
        case 404:
            response->desc = generate_str("Not Found");
            break;
        case 500:
            response->desc = generate_str("Internal Server Error");
            break;
        case 501:
            response->desc = generate_str("Not Implemented");
            break;
        case 502:
            response->desc = generate_str("Bad Gateway");
            break;
        case 503:
            response->desc = generate_str("Service Unavailable");
            break;
        default:
            return -1;
    }
    return 0;
}

char*
generate_str(const char* desc)
{
    size_t len = strlen(desc);
    char* tmp = (char*)malloc(sizeof(char)*(len+1));
    if (tmp != NULL) {
        strncpy(tmp, desc, len);
        tmp[len] = '\0';
    }
    return tmp;
}


#define code400 "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>400 Bad Request</title>\n</head><body>\n<h1>Bad Request</h1>\n<p>Your browser sent a request that this server could not understand.<br />\n</p>\n</body></html>\n"
#define code403 "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>403 Forbidden</h1>\n<p>Your browser sent a request that this server forbid.<br />\n</p>\n</body></html>\n"
#define code404 "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>404 Not Found</h1>\n<p>Your browser sent a request that this server could not find.<br />\n</p>\n</body></html>\n"
#define code500 "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>500 Internal Server Error</title>\n</head><body>\n<h1>500 Internal Server Error</h1>\n<p>Your browser sent a request that this server get error.<br />\n</p>\n</body></html>\n"

#define error_page(http_code) do {                                           \
    char str[20] = {0};                                                      \
    if (http_code != 400 && http_code != 403 &&                              \
        http_code != 404 && http_code != 500) {                              \
        break;                                                               \
    }                                                                        \
    size_t len = strlen(code##http_code);                                    \
    response->body = (char *)malloc(sizeof(char) * (len + 1));               \
    if (response->body != NULL) {                                            \
        strncpy(response->body, code##http_code, len);                       \
        response->body[len] = '\0';                                          \
		snprintf(str, sizeof(str), "%ld", (long int)strlen(response->body)); \
    } else {                                                                 \
		snprintf(str, sizeof(str), "%d", 0);                                 \
    }                                                                        \
    response_addfield(response, "Content-Length", 14, str, strlen(str));     \
    response_addfield(response, "Content-Type", 12, "text/html", 9);         \
} while( /* CONSTCOND */ 0 )

int
handleError(_response* response) {
    switch(response->code) {
        case 400:
            error_page(400);
            break;
        case 403:
            error_page(403);
            break;
        case 404:
            error_page(404);
            break;
        case 500:
            error_page(500);
            break;
        default:break;
    }
    return 0;
}

int
validate_request(/*Input*/const _request *request, /*Output*/_response *response)
{
    int ret = 0;
    if (request->errcode != NO_ERR) {
        if (request->errcode == SYSTEM_ERR)
            ret = 500;
        else
            ret = 400;
        return ret;
    }

    if (request->version == NULL ||
            request->uri == NULL) {
        ret = 400;
        return ret;
    }

    return ret;
}

int
validate_stat(/*Input*/const char *path, /*Output*/_response *response, struct stat *req_stat)
{
    int ret = 0;
    if (stat(path, req_stat) < 0) {
        if (errno == ENOENT)
            ret = 404;
        else if (errno == EACCES)
            ret = 403;
        else
            ret = 400;
        return ret;
    }

    if (access(path, R_OK) < 0) {
        if (errno == EACCES)
            ret = 403;
        else
            ret = 400;
        return ret;
    }
    return ret;
}

void
generate_response(int code, _response *response)
{
    response->code = code;
    generate_desc(response);
    if (code != 200 && code != 304)
        handleError(response);
}
