#include <sys/stat.h>

#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "index.h"
#include "public.h"

#define STR_HTML_DOCTYPE "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\"><html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en' lang='en'>"

static const char *
get_time_str(time_t ftime)
{
        static char buffer[32];
        strftime(buffer, 32, "%F", localtime(&ftime));
        return buffer;
}

char *
generate_index(const char *path)
{
    char *index_html, *pos;
    int left, count;
    struct dirent **dirlist;
    struct dirent *dp;
    int dircount, dirindex;
    struct stat sb;
    
    if ((index_html = (char *)malloc(INDEX_BUFFER_SIZE)) != NULL) {
        pos = index_html;
        left = INDEX_BUFFER_SIZE;

        if ((count = snprintf(pos, left, STR_HTML_DOCTYPE)) < 0) {
            WARN("generate_index(): write error");
            free(index_html);
            return NULL;
        } else {
            pos += count;
            left -= count;
        }

        if ((count = snprintf(pos, left, "<head><title>Path %s</title></head><body>Path %s<div><table class='list'><tr class='nohover'><th class='left'>Name</th><th class='right'>Size</th><th class='left'>Last Modified</th></tr>",
                        path, path)) < 0) {
            WARN("generate_index(): write error");
            free(index_html);
            return NULL;
        } else {
            pos += count;
            left -= count;
        }

        dircount = scandir(".", &dirlist, 0, alphasort);
        if (dircount < 0) {
            WARNP("generate_index(): can't open path");
            free(index_html);
            return NULL;
        } else {
            for (dirindex = 0; dirindex < dircount; ++dirindex) {
                dp = dirlist[dirindex];
                lstat(dp->d_name, &sb);
                if (dp->d_name[0] != '.') {
                    count = snprintf(pos, left, "<tr><td><a href='%s/%s'>%s</a></td><td>%lu</td><td>%s</td></tr>",
                                    path, dp->d_name, dp->d_name, sb.st_size, get_time_str(sb.st_mtime));
                    pos += count;
                    left -= count;
                }
                free(dirlist[dirindex]);
            }
            free(dirlist);
        }

        if ((count = snprintf(pos, left, "</table></div></body></html>")) < 0) {
            WARN("generate_index(): write error");
            free(index_html);
            return NULL;
        }
        index_html[INDEX_BUFFER_SIZE - 1] = '\0';
    }
    return index_html;
}
