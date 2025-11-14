#ifndef LINE_H
#define LINE_H

#include "utf8.h"
#include <stddef.h>

struct line {
    struct line* l_back;
    struct line* l_next;
    struct unicode_encoding* l_content;
    size_t l_size;
};

struct line* content_to_line(const char* content, size_t size);

struct line* l_alloc(const char* content, size_t begin, size_t end, struct line* lp_back);

void l_free(struct line* lp);

#endif // LINE_H