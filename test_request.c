#include "HTTP_parser.h"
#include "public.h"
#include "handle_cgi.h"
#include "handle_static.h"

int g_debug = 1;
FILE *g_log = NULL;
const char *g_dir = "./";
const char *g_dir_cgi = NULL;

/* test handling of static and cgi */
int
test_handling(char *filename)
{
//    char *buffer = 0;
//    long length;
//    FILE *f = fopen (filename, "rb");
//
//    if(f) {
//        fseek (f, 0, SEEK_END);
//        length = ftell (f);
//        fseek (f, 0, SEEK_SET);
//        buffer = malloc (length);
//        if (buffer) {
//            fread (buffer, 1, length, f);
//        }
//        fclose (f);
//    }


    _request *request = (_request *)malloc(sizeof(_request));
    _response *response = (_response *)malloc(sizeof(_response));
    request_init(request);
    response_init(response);

    request->uri = generate_str("/test_requests");
    request->version = generate_str("VERSION");

//    if (decode_request(buffer, request) != 0) {
//        perror("decode request error");
//        return -1;
//    }
    handle_static(request, response);
    printf("%s", encode_response(response));
    return 0;
}

int main(int argc, char *argv[]) {
    test_handling("./test_requests/request_static.txt");
    return 0;
}