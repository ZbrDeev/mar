#include "utf8.h"

struct unicode_encoding utf8_to_unicode(const unsigned char* line, size_t index, size_t len){
    struct unicode_encoding unicode_result;
    unicode_result.bytes_size = 1;
    unicode_result.result = line[index];

    unsigned char c = line[index];
    size_t bytes = 1;
    unsigned result = 0;

    if(c < 0xc0){
        return unicode_result;
    }

    int mask = 0x40;
    while(c & mask){
        ++bytes;
        mask >>= 1;
    }

    if(bytes > MAX_UTF8_SIZE || bytes > len){
        return unicode_result;
    }

    result = c & (mask - 1);

    for(size_t i = 1; i < bytes; ++i){
        c = line[index+i];

        if((c & 0xc0) != 0x80){
            return  unicode_result;
        }

        result = (result << 6) | (c & 0x3f);
    }

    unicode_result.result = result;
    unicode_result.bytes_size = bytes;

    return unicode_result;
}

struct utf8_encoding unicode_to_utf8(struct unicode_encoding unicode){
    struct utf8_encoding utf8;
    utf8.bytes_size = unicode.bytes_size;

    if(unicode.bytes_size == 1){
        utf8.result[0] =  unicode.result;
        return utf8;
    }

    utf8.result[0] = 0b11110000 << (3-(unicode.bytes_size-1));
    utf8.result[0] |= (unicode.result >> 6 * (unicode.bytes_size-1)) & (0x3f >> (unicode.bytes_size-1));


    for(size_t i = 1; i < unicode.bytes_size; ++i){
        utf8.result[i] =  unicode.result >> 6*(unicode.bytes_size-i-1) & 0x3f;
        utf8.result[i] |= 0x80;
    }

    return utf8;
}
