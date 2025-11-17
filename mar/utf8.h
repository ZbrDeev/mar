#ifndef UTF8_H
#define UTF8_H

#include <stddef.h>

#define MAX_UTF8_SIZE 4

struct __attribute__((packed)) unicode_encoding {
    unsigned bytes_size;
    unsigned result;
};

struct __attribute__((packed)) utf8_encoding {
    unsigned bytes_size;
    unsigned char result[MAX_UTF8_SIZE];
};

struct unicode_encoding utf8_to_unicode(const unsigned char* line, size_t index, size_t len);

struct utf8_encoding unicode_to_utf8(struct unicode_encoding unicode);

#endif // UTF8_H