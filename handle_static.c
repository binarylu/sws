#include "handle_static.h"

#define MAX_TIME_SIZE 100
#define SERVER_NAME "sws"
#define SERVER_NAME_SIZE 3
#define VERSION "HTTP/1.0"
#define BUFF_SIZE 1024

int
handle_static(/*Input*/const _request *request, /*Output*/_response *response)
{
    struct stat req_stat;
    char time_buff[MAX_TIME_SIZE];
    response->version = VERSION;
    get_date_rfc1123(time_buff, MAX_TIME_SIZE);
    response_addfield(response, "Date", 4, time_buff, MAX_TIME_SIZE);
    response_addfield(response, "Server", 6, SERVER_NAME, SERVER_NAME_SIZE);
    request->uri = get_absolute_path(request->uri, REQ_STATIC);

    if (stat(request->uri, req_stat) < 0) {
        if (strcmp(strerror(errno), ENOENT)) {
            response->code = 404;
            generate_desc(response);
        } else if (strcmp(strerror(errno), EACCES)){
            response->code = 403;
            generate_desc(response);
        } else {
            response->code = 400;
            generate_desc(response);
        }
        return 0;
    }
    if (validate_path_security(request->uri, REQ_STATIC)){
        response->code = 403;
        generate_desc(response);
        return 0;
    }

    
    if (if_modified(request, req_stat)) {
        response->code = 304;
        generate_desc(response);
        return 0;
    }

    if (S_ISREG(req_stat.st_mode)) {
        set_file(request, req_stat, response);
    } else if (S_ISDIR(req_stat.st_mode)) {
        set_directory(request, req_stat, response);
    }

    return 0;
}

int
if_modified(_request *request, struct stat req_stat)
{
    _head_entry head = request->header_entry;
    for (;head;head = head->next){
        if (!strcmp(head->key, "If-Modified-Since")){
            if (same_time(head->value, req_stat.st_mtime))
                return 1;
            else return 0;
        }
    }
    return 0;
}

int
same_time(char* val, time_t mtime)
{
    char time_buff[MAX_TIME_SIZE];
    struct tm *p;
    p = gmtime(mtime);
    
    strftime(time_buff, MAX_TIME_SIZE, "%a, %d %b %Y %H GMT", p);
    if (!strcmp(val, time_buff)) return 1;

    strftime(time_buff, MAX_TIME_SIZE, "%A, %d-%b-%y %H GMT", p);
    if (!strcmp(val, time_buff)) return 1;

    strftime(time_buff, MAX_TIME_SIZE, "%c", p);
    if (!strcmp(val, time_buff)) return 1;

    return 0;
}

int
set_file(_request *request, struct stat req_stat, _response *response)
{ 
    int req_fd;
    int nums;
    int buf[BUFF_SIZE];
    if ((req_fd = open(request->uri, O_RDONLY)) == -1 ) {
        response->code = 500;
        generate_desc(response);
        return 0;
    }
    
    while ((nums = read(req_fd, buf, BUFF_SIZE)) > 0) {
        
    }
    if (nums < 0) {
        response->code = 500;
        generate_desc(response);
        return 0;
    }
  
    // Close the files
    if (close(req_fd) == -1 ) {
        response->code = 500;
        generate_desc(response);
        return 0;
    }
}


int 
set_directory(_request *request, struct stat req_stat, _response *response)
{
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

