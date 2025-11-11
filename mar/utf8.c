#include "utf8.h"

struct unicode_encoding utf8_to_unicode(char* line, size_t *index, size_t len){
    struct unicode_encoding unicode_result;
    unicode_result.bytes_size = 1;
    unicode_result.result = line[*index];

    unsigned char c = line[*index];
    unsigned bytes = 1;
    unsigned result = 0;

    if(c < 0xc0){
        return unicode_result;
    }

    int mask = 0x40;
    while(c & mask){
        ++bytes;
        mask >>= 1;
    }

    if(bytes > 6 || bytes > len){
        return unicode_result;
    }

    for(size_t i = 1; i < bytes; ++i, ++(*index)){
        c = line[*index];

        if((c & 0xc0) != 0x80){
            return  unicode_result;
        }

        result = (result << 6) | (c & 0x3f);
    }

    unicode_result.result = result;
    unicode_result.bytes_size = bytes;

    return unicode_result;
}