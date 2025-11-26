#include "ansi.h"
#include "line.h"
#include "term.h"
#include "window.h"
#include <stdio.h>

void move_cursor(struct line_position position){
    putc(ESCAPE, stdout);
    printf("[%ld;%ldH", position.line+1, position.column+1);
    flush();
}

static void move_cursor_from_line(size_t line, size_t column){
    putc(ESCAPE, stdout);
    printf("[%ld;%ldH", line+1, column+1);
    flush();
}

void erase_line(void){
    putc(ESCAPE, stdout);
    printf("[2K");
}

void print_screen(struct line* lp, struct line_position position){
    clear_screen();
    struct line* lp_it = lp;
    struct line_position temp_position = {.line = 0, .column = 0};

    while(lp_it != NULL){
        print_line(lp_it, temp_position);
        ++temp_position.line;

        lp_it = lp_it->l_next;
    }

    move_cursor(position);
}

void print_line(struct line* lp, struct line_position position){
    move_cursor_from_line(position.line, 0);

    struct unicode_column* unicode_it = lp->l_content;

    for(size_t i = 0; i < lp->l_size; ++i){
        if(unicode_it == NULL)
            break;
        
        tputc(unicode_it->unicode);

        unicode_it = unicode_it->u_next;
    }

    putc('\n', stdout);

    move_cursor(position);
}

void clear_screen(void){
    putc(ESCAPE, stdout);
    printf("[2J");
}
