#include "HTTP_parser.h"
#include "public.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define CRLF    "\r\n"
#define SP_STR  " "
#define CR      '\r'
#define LF      '\n'
#define SP      ' '

static int
try_match(const char **pstr, const char *match, int count)
{
    assert(pstr && *pstr && match);
    assert(strlen(match) <= count);
    if (strncmp(*pstr, match, count) == 0) {
        *pstr += count;
        return 1;
    } else
        return 0;
}

static char *
try_capture(const char **pstr, char c)
{
    const char *loc;
    char *buf;
    int count;

    buf = NULL;
    count = 0;
    if ((loc = strchr(*pstr, c)) != NULL) {
        count = loc - *pstr;
        if ((buf = (char *)malloc(count + 1)) != NULL) {
            strncpy(buf, *pstr, count);
            buf[count] = '\0';
            *pstr = loc;
        } else
            perror("can't alllocate memory");
    }
    return buf;
}

static const char *
request_line(const char *str, _request *req)
{
    if (try_match(&str, "GET", 3))
        req->method = GET_METHOD;
    else if (try_match(&str, "HEAD", 4))
        req->method = HEAD_METHOD;
    else
        return NULL;

    if (try_match(&str, SP_STR, 1) &&
            (req->uri = try_capture(&str, SP)) != NULL &&
            try_match(&str, SP_STR, 1) &&
            (req->version = try_capture(&str, CR)) != NULL &&
            try_match(&str, CRLF, 2))
        return str;
    else {
        request_clear(req);
        return NULL;
    }
}

static const char *
digit(const char *str, int count)
{
    for (; count > 0; --count) {
        if (isdigit(*str))
            ++str;
        else {
            str = NULL;
            break;
        }
    }
    return str;
}

static const char *
_month(const char *str)
{
    static const char * strmonth[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    int i;
    for (i = 0; i < 12; ++i) {
        if (try_match(&str, strmonth[i], 3))
            break;
    }
    return i < 12 ? str : NULL;
}

static const char *
_wkday(const char *str)
{
    static const char * strwk[] = {
        "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
    };
    int i;
    for (i = 0; i < 7; ++i) {
        if (try_match(&str, strwk[i], 3))
            break;
    }
    return i < 7 ? str : NULL;
}

static const char *
_weekday(const char *str)
{
    static const char * strweek[] = {
        "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"
    };
    static const int lenweek[] = {6, 7, 9, 8, 6, 8, 6};
    int i;
    for (i = 0; i < 7; ++i) {
        if (try_match(&str, strweek[i], lenweek[i]))
            break;
    }
    return i < 7 ? str : NULL;
}

static const char *
_time(const char *str)
{
    if ((str = digit(str, 2)) != NULL &&
            try_match(&str, ":", 1) &&
            (str = digit(str, 2)) != NULL &&
            try_match(&str, ":", 1) &&
            (str = digit(str, 2)) != NULL)
        return str;
    else
        return NULL;
}

static const char *
_date1(const char *str)
{
    if ((str = digit(str, 2)) != NULL &&
            try_match(&str, SP_STR, 1) &&
            (str = _month(str)) != NULL &&
            try_match(&str, SP_STR, 1) &&
            (str = digit(str, 4)) != NULL)
        return str;
    else
        return NULL;
}

static const char *
_date2(const char *str)
{
    if ((str = digit(str, 2)) != NULL &&
            try_match(&str, "-", 1) &&
            (str = _month(str)) != NULL &&
            try_match(&str, "-", 1) &&
            (str = digit(str, 2)) != NULL)
        return str;
    else
        return NULL;
}

static const char *
_date3(const char *str)
{
    if ((str = _month(str)) != NULL &&
            try_match(&str, SP_STR, 1) &&
            (try_match(&str, SP_STR, 1) || (str = digit(str, 1)) != NULL) &&
            (str = digit(str, 1)) != NULL)
        return str;
    else
        return NULL;
}

static const char *
rfc1123_date(const char *str)
{
    if ((str = _wkday(str)) != NULL &&
            try_match(&str, "," SP_STR, 2) &&
            (str = _date1(str)) != NULL &&
            try_match(&str, SP_STR, 1) &&
            (str = _time(str)) != NULL &&
            try_match(&str, SP_STR "GMT", 4))
        return str;
    else
        return NULL;
}

static const char *
rfc850_date(const char *str)
{
    if ((str = _weekday(str)) != NULL &&
            try_match(&str, "," SP_STR, 2) &&
            (str = _date2(str)) != NULL &&
            try_match(&str, SP_STR, 1) &&
            (str = _time(str)) != NULL &&
            try_match(&str, SP_STR "GMT", 4))
        return str;
    else
        return NULL;
}

static const char *
asctime_date(const char *str)
{
    if ((str = _wkday(str)) != NULL &&
            try_match(&str, SP_STR, 1) &&
            (str = _date3(str)) != NULL &&
            try_match(&str, SP_STR, 1) &&
            (str = _time(str)) != NULL &&
            try_match(&str, SP_STR, 1) &&
            (str = digit(str, 4)) != NULL)
        return str;
    else
        return NULL;
}

static const char *
http_date(const char *str)
{
    const char *ret;

    ret = rfc1123_date(str);
    if (ret == NULL)
        ret = rfc850_date(str);
    if (ret == NULL)
        ret = asctime_date(str);
    return ret;
}

static const char *
general_header(const char *str, _request *req)
{
    const char *end;
    if (try_match(&str, "Date:" SP_STR, 6) &&
            (end = http_date(str)) != NULL &&
            request_addfield(req, "Date", 4, str, end - str) == 0 &&
            try_match(&end, CRLF, 2))
        return end;
    else
        return NULL;
}

static const char *
if_modified_since(const char *str)
{
    return http_date(str);
}

static const char *
request_header(const char *str, _request *req)
{
    const char *end;
    if (try_match(&str, "If-Modified-Since:" SP_STR, 19) &&
            (end = if_modified_since(str)) != NULL &&
            request_addfield(req, "If-Modified-Since", 17, str, end - str) == 0 &&
            try_match(&end, CRLF, 2))
        return end;
    else
        return NULL;
}

static const char *
skip_header(const char *str)
{
    if (try_match(&str, CRLF, 2))
        return NULL;

    str = strstr(str, CRLF);
    if (str != NULL && try_match(&str, CRLF, 2))
        return str;
    else
        return NULL;
}

int
decode_request(/*Input*/const char *content, /*Output*/_request *request)
{
    const char *progress;
    int more;

    if ((progress = request_line(content, request)) != NULL)
        content = progress;
    else
        return -1;
    do {
        more = 0;
        if ((progress = general_header(content, request)) != NULL) {
            content = progress;
            more = 1;
        }
        if ((progress = request_header(content, request)) != NULL) {
            content = progress;
            more = 1;
        }
        if (!more && (progress = skip_header(content)) != NULL) {
            content = progress;
            more = 1;
        }
    } while (more);
    if (try_match(&content, CRLF, 2))
        return 0;
    else {
        request_clear(request);
        return -1;
    }
}

char *
encode_response(/*Input*/const _response *response)
{
    int length, count;
    char *content, *pos;
    _header_entry *header;

    /* Status-Line */
    length = strlen(response->version) + 1 +
        3 + 1 +
        strlen(response->desc) + 2;

    /* Header */
    header = response->header_entry;
    while (header != NULL) {
        length += strlen(header->key) + 2 +
            strlen(header->value) + 2;
        header = header->next;
    }

    /* CRLF */
    length += 2;

    /* Entity Body */
    if (response->body != NULL)
        length += strlen(response->body);

    /* null terminator */
    length += 1;

    if ((content = (char *)malloc(length)) == NULL) {
        perror("can't allocate memory");
        return NULL;
    }
    pos = content;

    if ((count = snprintf(pos, length, "%s %u %s\r\n",
            response->version, response->code, response->desc)) < 0) {
        perror("write request error");
        free(content);
        return NULL;
    }
    length -= count;
    pos += count;

    header = response->header_entry;
    while (header != NULL) {
        if ((count = snprintf(pos, length, "%s: %s\r\n",
                        header->key, header->value)) < 0) {
            perror("write request error");
            free(content);
            return NULL;
        }
        length -= count;
        pos += count;
        header = header->next;
    }

    if (!response->is_cgi) {
        if ((count = snprintf(pos, length, "\r\n")) < 0) {
            perror("write request error");
            free(content);
            return NULL;
        }

        length -= count;
        pos += count;
    }

    if (response->body != NULL) {
        if(snprintf(pos, length, "%s", response->body) < 0) {
            perror("write request error");
            free(content);
            return NULL;
        }
    }

    return content;
}
