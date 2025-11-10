#ifndef UTF8_H
#define UTF8_H

#include <stddef.h>

struct __attribute__((packed)) unicode_encoding {
    unsigned bytes_size;
    unsigned result;
};

struct unicode_encoding utf8_to_unicode(char* line, size_t *index, size_t len);

#endif // UTF8_H