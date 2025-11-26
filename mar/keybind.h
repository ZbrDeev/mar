#ifndef KEYBIND_H
#define KEYBIND_H

#include "hashmap.h"
#include "line.h"
#include "window.h"

void init_keybind(void);

void read_key(struct window* wp);

#endif // KEYBIND_H