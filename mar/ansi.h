#ifndef ANSI_H
#define ANSI_H

#define ESCAPE 0x1b

void move_cursor(void);

void erase_line(void);

void clear_screen(void);

#endif // ANSI_H