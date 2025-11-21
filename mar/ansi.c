#include "ansi.h"
#include "line.h"
#include "term.h"
#include <stdio.h>

void move_cursor(size_t line, size_t column){
    putc(ESCAPE, stdout);
    printf("[%ld;%ldH", line+1, column+1);
    flush();
}

void erase_line(void){
    putc(ESCAPE, stdout);
    printf("[2K");
}

void print_screen(struct line* lp, size_t line, size_t column){
    clear_screen();
    struct line* lp_it = lp;

    size_t temp_line = 0;
    while(lp_it != NULL){
        print_line(lp_it, temp_line++, 0);

        lp_it = lp_it->l_next;
    }

    move_cursor(line, column);
}

void print_line(struct line* lp, size_t line, size_t column){
    move_cursor(line, 0);
    struct unicode_column* unicode_it = lp->l_content;

    for(size_t i = 0; i < lp->l_size; ++i){
        if(unicode_it == NULL)
            break;
        
        tputc(unicode_it->unicode);

        unicode_it = unicode_it->u_next;
    }

    putc('\n', stdout);

    move_cursor(line, column);
}

void clear_screen(void){
    putc(ESCAPE, stdout);
    printf("[2J");
}
