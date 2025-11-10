#ifndef LINE_H
#define LINE_H

#include <stddef.h>

struct line {
    struct line* l_back;
    struct line* l_next;
    char* l_content;
    size_t l_size;
};

struct line* content_to_line(const char* content, size_t size);

struct line* l_alloc(char* content, size_t size);

void l_free(struct line* lp);

#endif // LINE_H