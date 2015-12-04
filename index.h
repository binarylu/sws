#ifndef _INDEX_H_
#define _INDEX_H_

#define INDEX_BUFFER_SIZE (8 * 4096)

/* generate a html page of directory index */
char * generate_index(const char *path, const char *url);

#endif /* !_INDEX_H_ */
