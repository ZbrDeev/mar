#include "ansi.h"
#include "line.h"
#include "term.h"
#include <stdio.h>

void move_cursor(size_t line, size_t column){
    putc(ESCAPE, stdout);
    printf("[%ld;%ldH", line, column);
    flush();
}

void erase_line(void){
    putc(ESCAPE, stdout);
    printf("[K");
}

void print_line(struct line* lp, size_t line, size_t column){
    erase_line();
    move_cursor(line, 1);
    struct unicode_column* unicode_it = lp->l_content;

    for(size_t i = 0; i < lp->l_size; ++i){
        if(unicode_it == NULL)
            break;
        

        tputc(unicode_it->unicode);

        unicode_it = unicode_it->u_next;
    }

    move_cursor(line, column);
}

void clear_screen(void){
    putc(ESCAPE, stdout);
    printf("[2J");
}
