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