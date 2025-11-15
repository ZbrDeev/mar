#ifndef HASHMAP_H
#define HASHMAP_H

#define MAX_CAPACITY 10
#define CALC_KEY_INDEX(key) (key % MAX_CAPACITY)

typedef void (hashmap_function)(void);

struct node {
    unsigned key;
    hashmap_function* function;

    struct node* next;
};

struct hashmap {
    struct node** nodes;
};

struct hashmap init_hashmap(void);

struct node* navigate_in_node_hashmap(struct node* np, unsigned key);

struct node* get_value_hashmap(struct hashmap* hp, unsigned key);

void insert_value_hashmap(struct hashmap* hp, unsigned key, hashmap_function* function);

void free_hashmap(struct hashmap* hp);

#endif // HASHMAP_H