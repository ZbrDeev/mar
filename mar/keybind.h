#ifndef KEYBIND_H
#define KEYBIND_H

#include "hashmap.h"
#include "line.h"

#define MAX_KEY_SIZE 4

static struct hashmap keybind_hashmap;

void init_keybind(void);

void read_key(struct line* lp);

#endif // KEYBIND_H