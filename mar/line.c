#include "line.h"
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
            char *line_content;
            size_t line_size = i - start_content;

            if((line_content = malloc(line_size+1)) == NULL){
                printf("Failted to alloc line content\n");
                return NULL;
            }

            strncpy(line_content, &content[start_content], line_size);
            line_content[line_size] = '\0';

            lp_it->l_next = l_alloc(line_content, line_size, lp_it);
            lp_it = lp_it->l_next;
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

struct line* l_alloc(char* content, size_t size, struct line* lp_back){
    struct line* lp;

    if((lp = (struct line*)malloc(sizeof(struct line))) == NULL){
        printf("Failted to alloc line struct\n");
        return NULL;
    }

    lp->l_content = content;
    lp->l_size = size;
    lp->l_next = NULL;
    lp->l_back = lp_back;
    return lp;
}

void l_free(struct line* lp){
    if(lp != NULL && lp->l_next != NULL){
        l_free(lp->l_next);
    }

    free(lp->l_content);
    lp->l_content = NULL;

    free(lp);
    lp = NULL;
}