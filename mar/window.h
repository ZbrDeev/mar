#ifndef WINDOW_H
#define WINDOW_H

#include "line.h"

struct line_position {
    size_t line;
    size_t column;
};

struct window {
    char* filename;
    struct line* first_lp;
    struct line* current_lp;
    struct line_position position;
    size_t line_size;
    struct window* w_next;
    struct window* w_back;
};

struct window* alloc_window(void);

void render_window(struct window* wp);

void free_window(struct window* wp);

#endif // WINDOW_H