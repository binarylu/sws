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
            response->desc = "OK";
            break;
        case 201:
            response->desc = "Created";
            break;
        case 202:
            response->desc = "Accepted";
            break;
        case 204:
            response->desc = "No Content";
            break;
        case 300:
            response->desc = "Multiple Choices";
            break;
        case 301:
            response->desc = "Moved Permanently";
            break;
        case 302:
            response->desc = "Moved Temporarily";
            break;
        case 304:
            response->desc = "Not Modified";
            break;
        case 400:
            response->desc = "Bad Request";
            break;
        case 401:
            response->desc = "Unauthorized";
            break;
        case 403:
            response->desc = "Forbidden";
            break;
        case 404:
            response->desc = "Not Found";
            break;
        case 500:
            response->desc = "Internal Server Error";
            break;
        case 501:
            response->desc = "Not Implemented";
            break;
        case 502:
            response->desc = "Bad Gateway";
            break;
        case 503:
            response->desc = "Service Unavailable";
            break;
        default:
            return -1;
    }
    return 0;
}
