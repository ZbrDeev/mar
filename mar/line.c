#include "line.h"
#include "utf8.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct line* content_to_line(const unsigned char* content, size_t size){
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
        ++line_size;
    }

    lp_it = lp->l_next;
    lp_it->l_back = NULL;
    free(lp);
    lp = NULL;

    return lp_it;
}

static void line_to_unicode(const unsigned char* content, size_t begin, size_t end, struct line* lp){
    struct unicode_column* unicodes = (struct unicode_column*)malloc(sizeof(struct unicode_column));
    assert(unicodes != NULL);

    unicodes->u_back = NULL;
    unicodes->u_next = NULL;

    struct unicode_column* unicode_it = unicodes;

    size_t index = 0;
    size_t size = end - begin;

    while(index < size){
        ++lp->l_size;
        struct unicode_encoding result = utf8_to_unicode(&content[begin], index, size);
        index += result.bytes_size;

        struct unicode_column* temp = unicode_it;

        unicode_it = unicode_it->u_next = (struct unicode_column*)malloc(sizeof(struct unicode_column));
        unicode_it->unicode = result;

        unicode_it->u_back = temp;
        unicode_it->u_next = NULL;
    }

    lp->l_last_content = unicode_it;
    
    unicode_it = unicodes->u_next;
    unicode_it->u_back = NULL;
    free(unicodes);
    unicodes = NULL;

    lp->l_content = unicode_it;
}

struct line* l_alloc(const unsigned char* content, size_t begin, size_t end, struct line* lp_back){
    struct line* lp;

    if((lp = (struct line*)malloc(sizeof(struct line))) == NULL){
        printf("Failed to alloc line struct\n");
        return NULL;
    }

    lp->l_size = 0;
    lp->l_next = NULL;
    lp->l_back = lp_back;
    line_to_unicode(content, begin, end, lp);

    return lp;
}

static void insert_unicode_node(struct unicode_column* up, struct unicode_encoding unicode){
    struct unicode_column* temp = (struct unicode_column*)malloc(sizeof(struct unicode_column));

    temp->unicode = unicode;
    temp->u_next = up;
    temp->u_back = up->u_back;

    if(up->u_back != NULL){
        up->u_back->u_next = temp;
    }
    
    up->u_back = temp;
}

void insert_char_in_line(struct line* lp, struct unicode_encoding unicode, size_t column){
    ++lp->l_size;
    struct unicode_column* unicode_it = lp->l_content;

    size_t i = 0;

    for(i = 0; i < column; ++i){
        if(unicode_it->u_next == NULL){
            insert_unicode_node(unicode_it, unicode);

            return;
        }

        unicode_it = unicode_it->u_next;
    }
    
    insert_unicode_node(unicode_it, unicode);

    if(i == 0)
        lp->l_content = lp->l_content[0].u_back;
}

static void free_unicode(struct unicode_column* up){
    if(up->u_next != NULL)
        free_unicode(up->u_next);
    

    free(up);
    up = NULL;
}

void l_free(struct line* lp){
    if(lp->l_next != NULL)
        l_free(lp->l_next);
    

    free_unicode(lp->l_content);

    free(lp);
    lp = NULL;
}