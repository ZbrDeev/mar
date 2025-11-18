#ifndef HASHMAP_H
#define HASHMAP_H

#define MAX_CAPACITY 10
#define CALC_KEY_INDEX(key) (key % MAX_CAPACITY)

typedef void (hashmap_function_t)(void);

struct node {
    unsigned key;
    hashmap_function_t* function;

    struct node* next;
};

struct hashmap {
    struct node* nodes[MAX_CAPACITY];
};

struct hashmap h_init(void);

struct node* h_navigate_in_node(struct node* np, unsigned key);

struct node* h_get_value(struct hashmap* hp, unsigned key);

void h_insert_value(struct hashmap* hp, unsigned key, hashmap_function_t* function);

void h_free(struct hashmap* hp);

#endif // HASHMAP_H