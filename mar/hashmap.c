#include "hashmap.h"
#include <stdlib.h>

struct hashmap init_hashmap(void){
    struct hashmap hash;

    hash.nodes = (struct node**)malloc(sizeof(struct node*));

    for(size_t i = 0; i < MAX_CAPACITY; ++i){
        hash.nodes[i] = NULL;
    }

    return hash;
}

struct node* navigate_in_node_hashmap(struct node* np, unsigned key){
    while(np != NULL && np->next != NULL){
        if(np->key == key){
            return np;
        }

        np = np->next;
    }

    return NULL;
}

struct node* get_value_hashmap(struct hashmap* hp, unsigned key){
    unsigned key_hashed = CALC_KEY_INDEX(key);
    struct node* np = hp->nodes[key_hashed];

    while(np->next != NULL){
        if(np->key == key){
            return np;
        }

        np = np->next;
    }

    return NULL;
}

void insert_value_hashmap(struct hashmap* hp, unsigned key, hashmap_function* function){
    unsigned key_hashed = CALC_KEY_INDEX(key);
    struct node* np = hp->nodes[key_hashed];

    while(np->next != NULL || np->key == key){
        np = np->next;
    }

    
    if(np == NULL){
        np = (struct node*)malloc(sizeof(struct node));
        np->next = NULL;
    }

    np->key = key;
    np->function = function;
}

void free_hashmap(struct hashmap* hp){
    for(size_t i = 0; i < MAX_CAPACITY; ++i){
        if(hp->nodes[i] != NULL){
            free(hp->nodes[i]);
        }
    }

    free(hp->nodes);
}