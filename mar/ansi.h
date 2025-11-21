#ifndef ANSI_H
#define ANSI_H

#include "line.h"

#define ESCAPE 0x1b
#define DELETE_KEY 127
#define ENTER_KEY 13

void move_cursor(size_t line, size_t column);

void erase_line(void);

void print_screen(struct line* lp, size_t line, size_t column);

void print_line(struct line* lp, size_t line, size_t column);

void clear_screen(void);

#endif // ANSI_H