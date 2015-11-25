#include "handle_response.h"

void
respond_not_found(_response *resp)
{
    resp->code = 404;
    resp->desc = "Not Found";
}

void
respond_forbidden(_response *resp)
{
    resp->code = 403;
    resp->desc = "Forbidden";
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
    strncpy(tmp, desc, len+1);
    return tmp;
}
