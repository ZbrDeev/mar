#ifndef KEYBIND_H
#define KEYBIND_H

#include "hashmap.h"
#include "line.h"
#include <stdio.h>

#define MAX_KEY_SIZE 4

void init_keybind(void);

void read_key(struct line* lp, FILE* fp);

#endif // KEYBIND_H