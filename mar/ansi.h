#ifndef ANSI_H
#define ANSI_H

#include "line.h"
#include "window.h"

#define ESCAPE 0x1b
#define CSI 0x5b
#define CTRL(key) key - 96
#define DELETE_KEY 127
#define ENTER_KEY 13

void move_cursor(struct line_position position);

void set_color(int color);

void reset_color(void);

void print_screen(struct window* wp);

void print_line(struct line* lp, struct line_position position, struct selected_text selected);

void clear_screen(void);

#endif // ANSI_H