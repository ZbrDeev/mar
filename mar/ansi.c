#include "ansi.h"
#include "line.h"
#include "term.h"
#include "window.h"
#include <stdio.h>

void move_cursor(struct line_position position){
    putc(ESCAPE, stdout);
    putc(CSI, stdout);
    printf("%ld;%ldH", position.line+1, position.column+1);
    flush();
}

static void move_cursor_from_line(size_t line, size_t column){
    putc(ESCAPE, stdout);
    putc(CSI, stdout);
    printf("%ld;%ldH", line+1, column+1);
    flush();
}

void set_color(int color){
    putc(ESCAPE, stdout);
    putc(CSI, stdout);
    printf("%dm", color);
}

void reset_color(void){
    putc(ESCAPE, stdout);
    putc(CSI, stdout);
    printf("49m");
}

void print_screen(struct window* wp){
    clear_screen();
    struct line* lp_it = wp->first_lp;
    struct line_position temp_position = {.line = 0, .column = 0};

    while(lp_it != NULL){
        print_line(lp_it, temp_position, wp->selected);
        ++temp_position.line;

        lp_it = lp_it->l_next;
    }

    move_cursor(wp->position);
}

void print_line(struct line* lp, struct line_position position, struct selected_text selected){
    move_cursor_from_line(position.line, 0);

    struct unicode_column* unicode_it = lp->l_content;

    for(size_t i = 0; i < lp->l_size; ++i){
        if(unicode_it == NULL)
            break;

        if(selected.is_selected && position.line == selected.line && (i >= selected.start && i < selected.end))
            set_color(100);
        else
            reset_color();
        
        
        tputc(unicode_it->unicode);

        unicode_it = unicode_it->u_next;
    }

    putc('\n', stdout);

    move_cursor(position);
}

void clear_screen(void){
    putc(ESCAPE, stdout);
    putc(CSI, stdout);
    printf("2J");
}
