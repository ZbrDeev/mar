#include "line.h"
#include "utf8.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct line* content_to_line(const unsigned char* content, size_t size){
    struct line* lp; 

    if((lp = malloc(sizeof(struct line))) == NULL){
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
            if(start_content < i)
                lp_it = lp_it->l_next = l_alloc(content, start_content, i, lp_it);
            
            break;
        }

        ++i;
    }

    if(lp->l_next != NULL){
        lp_it = lp->l_next;
        lp_it->l_back = NULL;
        free(lp);
        lp = NULL;
    }

    return lp_it;
}

static void line_to_unicode(const unsigned char* content, size_t begin, size_t end, struct line* lp){
    struct unicode_column* unicodes = malloc(sizeof(struct unicode_column));
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

        unicode_it = unicode_it->u_next = malloc(sizeof(struct unicode_column));
        unicode_it->unicode = result;

        unicode_it->u_back = temp;
        unicode_it->u_next = NULL;
    }

    lp->l_last_content = unicode_it;
    
    unicode_it = unicodes->u_next;

    if(unicode_it != NULL)
        unicode_it->u_back = NULL;
    
    free(unicodes);
    unicodes = NULL;

    lp->l_content = unicode_it;
}

struct line* l_alloc(const unsigned char* content, size_t begin, size_t end, struct line* lp_back){
    struct line* lp;

    if((lp = malloc(sizeof(struct line))) == NULL){
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
    struct unicode_column* temp = malloc(sizeof(struct unicode_column));

    temp->unicode = unicode;
    temp->u_next = NULL;
    temp->u_back = NULL;

    if(up->u_next != NULL){
        temp->u_next = up->u_next;
        up->u_next->u_back = temp;
    }
    
    up->u_next = temp;
    temp->u_back = up;
}

void insert_char_in_line(struct line* lp, struct unicode_encoding unicode, size_t column){
    ++lp->l_size;
    struct unicode_column* unicode_it = lp->l_content;

    if(unicode_it == NULL){
        lp->l_content = malloc(sizeof(struct unicode_column));
        lp->l_content->unicode = unicode;
        lp->l_content->u_back = NULL;
        lp->l_content->u_next = NULL;
        lp->l_last_content = lp->l_content;
        lp->l_size = 1;

        return;
    }

    if(column == 0){
        struct unicode_column* temp = malloc(sizeof(struct unicode_column));
        temp->unicode = unicode;
        temp->u_next = lp->l_content;
        temp->u_back = NULL;

        lp->l_content->u_back = temp;
        lp->l_content = temp;

        return;
    }

    for(size_t i = 0; i < column-1; ++i){
        if(unicode_it->u_next == NULL){
            insert_unicode_node(unicode_it, unicode);
            return;
        }

        unicode_it = unicode_it->u_next;
    }
    
    insert_unicode_node(unicode_it, unicode);
}

char* return_copied_char(struct line* lp, size_t start, size_t end, size_t* returned_size){
    size_t selected_size = end-start;
    char* selected_char = malloc(1);
    size_t content_size = 0;

    struct unicode_column* unicode_it = lp->l_content;

    for(size_t i = 0; i < start; ++i){
        if(unicode_it->u_next == NULL)
            break;

        unicode_it = unicode_it->u_next;
    }

    for(size_t i = start; i < end; ++i){
        if(unicode_it == NULL)
            break;
        

        struct utf8_encoding utf8 = unicode_to_utf8(unicode_it->unicode);

        content_size += utf8.bytes_size;

        char* temp = realloc(selected_char, content_size);
        assert(temp != NULL);
        selected_char = temp;

        for(size_t j = 0; j < utf8.bytes_size; ++j){
            size_t index = content_size - utf8.bytes_size + j;
            selected_char[index] = utf8.result[j];
        }

        unicode_it = unicode_it->u_next;
    }

    *returned_size = selected_size;

    return selected_char;
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
    

    if(lp->l_content != NULL){
        free_unicode(lp->l_content);
    }

    free(lp);
    lp = NULL;
}