#include "public.h"
#include <stdlib.h>
#include <bsd/string.h>

/* initialize struct */
void
request_init(_request *req)
{
    req->head_entry = NULL;
    req->uri = req->version = NULL;
    req->method = NONE_METHOD;
}

static void
free_entries(_header *h)
{
    if (h != NULL) {
        free_entries(h->next);
        free(h->key);
        free(h->value);
        free(h);
    }
}

/* release memory and reset */
void
request_clear(_request *req)
{
    if (req->uri != NULL)
        free(req->uri);
    if (req->version != NULL)
        free(req->version);
    if (req->head_entry != NULL)
        free_entries(req->head_entry);
    request_init(req);
}

/* return 0 on success */
int
request_addfield(_request *req, const char *key,
        int keylen, const char *val, int vallen)
{
    _header *last;

    last = req->head_entry;
    if (last == NULL) {
        if ((last = (_header *)malloc(sizeof(_header))) == NULL)
            return -1;
        req->head_entry = last;
    } else {
        while (last->next != NULL)
            last = last->next;
        if ((last->next = (_header *)malloc(sizeof(_header))) == NULL)
            return -1;
        last = last->next;
    }

    if ((last->key = (char *)malloc(keylen + 1)) == NULL) {
        free(last);
        return -1;
    }
    if ((last->value = (char *)malloc(vallen + 1)) == NULL) {
        free(last->key);
        free(last);
        return -1;
    }
    strlcpy(last->key, key, keylen + 1);
    strlcpy(last->value, val, vallen + 1);
    return 0;
}

