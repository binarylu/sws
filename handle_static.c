#include "handle_static.h"

int
handle_static(/*Input*/_request *request, /*Output*/_response *response)
{
    struct stat* req_stat;
    char str[20];
    char time_buff[MAX_TIME_SIZE];
    struct tm *p;

    response->version = VERSION;
    get_date_rfc1123(time_buff, MAX_TIME_SIZE);
    response_addfield(response, "Date", 4, time_buff, strlen(time_buff));
    response_addfield(response, "Server", 6, SERVER_NAME, SERVER_NAME_SIZE);
    request->uri = get_absolute_path(request->uri, REQ_STATIC);

    if (stat(request->uri, req_stat) < 0) {
        if (errno == ENOENT) {
            response->code = 404;
            generate_desc(response);
        } else if (errno == EACCES){
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
    
    p = gmtime(&(req_stat->st_mtime));
    strftime(time_buff, MAX_TIME_SIZE, "%a, %d %b %Y %H GMT", p);
    response_addfield(response, "Last-Modified", 13, time_buff, strlen(time_buff));

    sprintf(str, "%lld", req_stat->st_size);
    response_addfield(response, "Content-Length", 14, str, strlen(str));

    if (S_ISREG(req_stat->st_mode)) {
        if (set_file(request, req_stat, response) == 0)
            return 0;
    } else if (S_ISDIR(req_stat->st_mode)) {
        if (set_directory(request, req_stat, response) == 0)
            return 0;
    }

    response->code = 500;
    generate_desc(response);
    return -1;
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
    
    strftime(time_buff, MAX_TIME_SIZE, "%a, %d %b %Y %H GMT", p);
    if (strcmp(val, time_buff) == 0) return 1;

    strftime(time_buff, MAX_TIME_SIZE, "%A, %d-%b-%y %H GMT", p);
    if (strcmp(val, time_buff) == 0) return 1;

    strftime(time_buff, MAX_TIME_SIZE, "%c", p);
    if (strcmp(val, time_buff) == 0) return 1;

    return 0;
}

int
set_file(const _request *request, const struct stat* req_stat, _response *response)
{ 
    int req_fd;
    int nums;
    const char* mime;
    char buf[BUFF_SIZE];
    int body_size;

    mime = getMIME(request->uri);
    response_addfield(response, "Content-Type", 12, mime, strlen(mime));

    body_size = req_stat->st_size;
    response->body = (char *)malloc(body_size);
    if ((req_fd = open(request->uri, O_RDONLY)) == -1 ) {
        return -1;
    }
    while ((nums = read(req_fd, buf, BUFF_SIZE)) > 0) {
        if (strncpy(response->body, buf, nums) != 0){
            return -1;
        }
    }
    if (nums < 0) {
        return -1;
    }
    if (close(req_fd) == -1 ) {
        return -1;
    }
    return 0;
}


int 
set_directory(const _request *request, struct stat* req_stat, _response *response)
{
    DIR *dirp;
    struct dirent *dp;
    int len;
    char *path;
    const char* mime;

    dirp = opendir(request->uri);
    len = strlen(INDEX);
    while ((dp = readdir(dirp)) != NULL) {
            if (dp->d_namlen == len && strcmp(dp->d_name, INDEX) == 0) {
                path = request->uri;   
                if (path[strlen(path)-1] != '/')
                    strcat(path, "/");
                strcat(path, INDEX);
                (void)closedir(dirp);
                return set_file(request, req_stat, response);
            }
    }
    (void)closedir(dirp);

    mime = getMIME(request->uri);
    response_addfield(response, "Content-Type", 12, mime, strlen(mime));

    return 0;
}

const char*
getMIME(const char* path)
{
    const char *mime;
    magic_t magic;

    magic = magic_open(MAGIC_MIME_TYPE); 
    magic_load(magic, NULL);
    magic_compile(magic, NULL);
    mime = magic_file(magic, path);
    magic_close(magic);

    return mime;
}

