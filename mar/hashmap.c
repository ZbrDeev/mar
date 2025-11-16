#include "hashmap.h"
#include <stdlib.h>

struct hashmap h_init(void){
    struct hashmap hash;

    hash.nodes = (struct node**)malloc(sizeof(struct node*) * MAX_CAPACITY);

    for(size_t i = 0; i < MAX_CAPACITY; ++i){
        hash.nodes[i] = NULL;
    }

    return hash;
}

struct node* h_navigate_in_node(struct node* np, unsigned key){
    while(np != NULL && np->next != NULL){
        if(np->key == key){
            return np;
        }

        np = np->next;
    }

    return NULL;
}

struct node* h_get_value(struct hashmap* hp, unsigned key){
    unsigned key_hashed = CALC_KEY_INDEX(key);
    struct node* np = hp->nodes[key_hashed];

    if(np == NULL){
        return NULL;
    }

    while(np->next != NULL){
        if(np->key == key){
            return np;
        }

        np = np->next;
    }

    if(np->key == key){
        return np;
    }

    return NULL;
}

void h_insert_value(struct hashmap* hp, unsigned key, hashmap_function_t* function){
    unsigned key_hashed = CALC_KEY_INDEX(key);

    struct node* new_node = (struct node*)malloc(sizeof(struct node));
    new_node->key = key;
    new_node->function = function;
    new_node->next = NULL;

    
    if(hp->nodes[key_hashed] == NULL){
        hp->nodes[key_hashed] = new_node;
    }else{
        new_node->next = hp->nodes[key_hashed];
        hp->nodes[key_hashed] = new_node;
    }
}

static void free_node(struct node* np){
    if(np == NULL){
        return;
    }

    free_node(np->next);
    free(np);
}

void h_free(struct hashmap* hp){
    for(size_t i = 0; i < MAX_CAPACITY; ++i){
        if(hp->nodes[i] != NULL){
            free_node(hp->nodes[i]);
        }
    }

    free(hp->nodes);
}