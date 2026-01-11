#ifndef WINDOW_H
#define WINDOW_H

#include "line.h"
#include <stdbool.h>

#define MAX_WINDOW 4

struct line_position {
    size_t line;
    size_t column;
};

enum last_direction_selected {
    left,
    right
};

struct selected_text {
    bool is_selected;
    enum last_direction_selected last_selected;
    size_t line;
    size_t start;
    size_t end;
};

struct window {
    char* filename;
    struct line* first_lp;
    struct line* current_lp;

    struct line_position position;
    struct selected_text selected;
    size_t y_cursor;

    size_t line_size;

    char* status_bar_text;

    struct window* w_next;
    struct window* w_back;
};

struct window* alloc_window(void);

void render_window(struct window* wp);

void free_window(struct window* wp);

#endif // WINDOW_H