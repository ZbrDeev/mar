#ifndef LINE_H
#define LINE_H

#include "utf8.h"
#include <stddef.h>

struct unicode_column {
    struct unicode_encoding unicode;
    struct unicode_column* u_back;
    struct unicode_column* u_next;
};

struct line {
    struct line* l_back;
    struct line* l_next;
    struct unicode_column* l_content;
    struct unicode_column* l_last_content;
    size_t l_size;
};


struct line* content_to_line(const unsigned char* content, size_t size);

struct line* l_alloc(const unsigned char* content, size_t begin, size_t end, struct line* lp_back);

void insert_char_in_line(struct line* lp, struct unicode_encoding unicode, size_t column);

char* return_copied_char(struct line* lp, size_t start, size_t end, size_t* returned_size);

void l_free(struct line* lp);

#endif // LINE_H