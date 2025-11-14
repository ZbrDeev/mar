#include "line.h"
#include "utf8.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct line* content_to_line(const char* content, size_t size){
    struct line* lp; 

    if((lp = (struct line*)malloc(sizeof(struct line))) == NULL){
        printf("Failed to alloc line\n");
        return NULL;
    }

    lp->l_content = NULL;
    lp->l_back = NULL;
    lp->l_next = NULL;

    size_t i = 0;
    size_t start_content = 0;
    struct line* lp_it = lp;

    while(i < size){
        if(content[i] == '\n'){
            lp_it = lp_it->l_next = l_alloc(content, start_content, i, lp_it);
            start_content = i + 1;
        } else if(content[i] == '\0'){
            // TODO: CHECK IF THERE IS STILL VALUES
            break;
        }

        ++i;
    }

    lp_it = lp->l_next;
    lp_it->l_back = NULL;
    free(lp);
    lp = NULL;

    return lp_it;
}

static struct unicode_encoding* content_to_unicode(const char* content, size_t begin, size_t end, size_t *real_size){
    struct unicode_encoding* unicodes = (struct unicode_encoding*)malloc(sizeof(struct unicode_encoding));
    assert(unicodes != NULL);

    size_t index = 0;
    size_t size = end - begin;

    while(index < size){
        ++(*real_size);
        struct unicode_encoding result = utf8_to_unicode(&content[begin], index, size);
        index += result.bytes_size;

        struct unicode_encoding* temp = (struct unicode_encoding*)realloc(unicodes, sizeof(struct unicode_encoding) * (*real_size));
        assert(temp != NULL);
        unicodes = temp;

        unicodes[(*real_size)-1] = result;
    }

    return unicodes;
}

struct line* l_alloc(const char* content, size_t begin, size_t end, struct line* lp_back){
    struct line* lp;

    if((lp = (struct line*)malloc(sizeof(struct line))) == NULL){
        printf("Failted to alloc line struct\n");
        return NULL;
    }

    size_t size = 0;
    lp->l_content = content_to_unicode(content, begin, end, &size);
    lp->l_size = size;
    lp->l_next = NULL;
    lp->l_back = lp_back;
    return lp;
}

void l_free(struct line* lp){
    if(lp->l_next != NULL){
        l_free(lp->l_next);
    }

    free(lp->l_content);
    lp->l_content = NULL;

    free(lp);
    lp = NULL;
}