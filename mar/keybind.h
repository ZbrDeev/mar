#ifndef KEYBIND_H
#define KEYBIND_H

#include "hashmap.h"

static struct hashmap keybind_hashmap;

void init_keybind(void);

void read_key(void);

#endif // KEYBIND_H