#ifndef ANSI_H
#define ANSI_H

#include "line.h"
#include "window.h"

#define ESCAPE 0x1b
#define DELETE_KEY 127
#define ENTER_KEY 13

void move_cursor(struct line_position position);

void erase_line(void);

void print_screen(struct line* lp, struct line_position position);

void print_line(struct line* lp, struct line_position position);

void clear_screen(void);

#endif // ANSI_H