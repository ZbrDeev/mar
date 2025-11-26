#include "window.h"
#include "ansi.h"
#include "line.h"
#include <stdlib.h>

struct window* alloc_window(void){
    struct window* wp;

    if((wp = (struct window*)malloc(sizeof(struct window))) == NULL)
        return NULL;
    
    wp->w_next = NULL;
    wp->w_back = NULL;
    wp->filename = NULL;
    wp->line_size = 0;
    wp->position.line = 0;
    wp->position.column = 0;
    
    return wp;
}

void render_window(struct window* wp){
    struct line_position temp_position = {.line = 0, .column = 0};
    clear_screen();
    move_cursor(temp_position);
    print_screen(wp->first_lp, temp_position);
}

static void free_window_node(struct window* wp){
    if(wp->w_next != NULL){
        free_window(wp->w_next);
    }

    l_free(wp->first_lp);
    free(wp);
    wp = NULL;
}

void free_window(struct window* wp){
    free_window_node(wp);
}