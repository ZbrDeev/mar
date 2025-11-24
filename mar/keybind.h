#ifndef KEYBIND_H
#define KEYBIND_H

#include "hashmap.h"
#include "line.h"

void init_keybind(void);

void read_key(struct line* lp, char* fp);

#endif // KEYBIND_H