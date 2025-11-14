#include "ansi.h"
#include "term.h"
#include <stdio.h>

void move_cursor(void){
    putc(ESCAPE, stdout);
    printf("[%d;%dH", line, column);
}

void erase_line(void){
    putc(ESCAPE, stdout);
    printf("[2K");
}

void clear_screen(void){
    putc(ESCAPE, stdout);
    printf("[2J");
}
