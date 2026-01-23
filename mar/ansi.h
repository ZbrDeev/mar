#ifndef ANSI_H
#define ANSI_H

#include "line.h"
#include "window.h"

#define ESCAPE 0x1b
#define CSI 0x5b
#define CTRL(key) key - 0x60
#define DELETE_KEY 0x7f
#define ENTER_KEY 0xd
#define TAB 0x9

#define MAX_LINE 65
#define MAX_COL 100

#define MAX_TEXT_LINE 60

void move_cursor(struct window* wp);

void temp_move_cursor(struct line_position position);

void erase_line(void);

void set_color(int color);

void reset_color(void);

void render_statbar(struct window* wp);

void render_status_bar(struct window* wp);

void render_message(struct window* wp, const char* message, bool is_input);

void print_screen(struct window* wp);

void print_line(struct window *wp);

void clear_screen(void);

#endif // ANSI_H