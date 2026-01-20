#include "window.h"
#include "ansi.h"
#include "line.h"
#include <assert.h>
#include <stdlib.h>

struct window* w_alloc(void){
    struct window* wp = malloc(sizeof(struct window));
    assert(wp != NULL);
    
    wp->w_next = NULL;
    wp->w_back = NULL;
    wp->filename = NULL;
    wp->status_bar_text = "";

    wp->line_size = 0;
    wp->position.line = 0;
    wp->position.column = 0;
    wp->y_cursor = 0;

    wp->selected.is_selected = false;
    wp->selected.start = 0;
    wp->selected.end = 0;
    wp->selected.line = 0;
    
    return wp;
}

void render_window(struct window* wp){
    clear_screen();
    move_cursor(wp);
    print_screen(wp);
}

static void free_window_node(struct window* wp){
    if(wp->w_next != NULL)
        free_window(wp->w_next);
    

    l_free(wp->first_lp);
    free(wp);
    wp = NULL;
}

void free_window(struct window* wp){
    free_window_node(wp);
}