#include "ansi.h"
#include "term.h"
#include "window.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void move_cursor(struct window* wp){
    putc(ESCAPE, stdout);
    putc(CSI, stdout);

    if(wp->position.line > MAX_TEXT_LINE)
        printf("%ld;%ldH", wp->y_cursor + 1, wp->position.column + 1);
    else
        printf("%ld;%ldH", wp->position.line + 1, wp->position.column + 1);

    flush();
}

void temp_move_cursor(struct line_position position){
    putc(ESCAPE, stdout);
    putc(CSI, stdout);
    printf("%ld;%ldH", position.line+1, position.column+1);
    flush();
}

void erase_line(void){
    putc(ESCAPE, stdout);
    printf("[K");
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

void render_statbar(struct window* wp){
    struct line_position new_position = {.line = MAX_TEXT_LINE+1, .column = 0};

    size_t box_text_size = snprintf(NULL, 0, "File: %s - Line: %ld - Column: %ld", wp->filename, wp->position.line, wp->position.column);
    char* box_text = malloc(box_text_size + 1);
    assert(box_text != NULL);
    snprintf(box_text, box_text_size+1, "File: %s - Line: %ld - Column: %ld", wp->filename, wp->position.line, wp->position.column);

    temp_move_cursor(new_position);

    printf("┌");
    for(size_t i = 0; i < box_text_size+2; ++i)
        printf("─");
    printf("┐");

    ++new_position.line;
    temp_move_cursor(new_position);
    printf("│ %s │", box_text);

    ++new_position.line;
    temp_move_cursor(new_position);

    printf("└");
    for(size_t i = 0; i < box_text_size+2; ++i)
        printf("─");
    printf("┘");

    move_cursor(wp);

    free(box_text);
}

void render_status_bar(struct window *wp){
    if(strlen(wp->status_bar_text) <= 0)
        return;

    struct line_position new_position = {.line = MAX_TEXT_LINE+4, .column = 0};

    temp_move_cursor(new_position);

    printf("%s", wp->status_bar_text);

    move_cursor(wp);
    wp->status_bar_text = "";
}

void render_message(struct window* wp, const char* message, bool is_input){
    struct line_position new_position = {.line = MAX_LINE, .column = 0};

    temp_move_cursor(new_position);
    printf("%s", message);

    if(!is_input)
        move_cursor(wp);
}

static void temp_print_line(struct line* lp, size_t y_cursor, size_t current_y_cursor, struct line_position position, struct selected_text selected){
    struct line_position temp_position = {.line = y_cursor, .column = position.column};
    struct unicode_column* unicode_it = lp->l_content;
    
    move_cursor_from_line(y_cursor, 0);

    for(size_t i = 0; i < lp->l_size; ++i){
        if(unicode_it == NULL)
            break;

        if(selected.is_selected && y_cursor == current_y_cursor && (i >= selected.start && i < selected.end))
            set_color(100);
        else
            reset_color();
        
        tputc(unicode_it->unicode);

        unicode_it = unicode_it->u_next;
    }

    putc('\n', stdout);
    temp_move_cursor(temp_position);
}

void print_screen(struct window* wp){
    struct line* lp_it = wp->first_lp;
    struct line_position temp_position = {.line = 0, .column = 0};
    size_t index = 0;
    size_t calc_y_offset = 0;

    clear_screen();
    temp_move_cursor(temp_position);

    if(wp->position.line > MAX_TEXT_LINE)
        calc_y_offset = wp->position.line - wp->y_cursor;

    for(size_t i = 0; i < calc_y_offset; ++i)
        lp_it = lp_it->l_next;

    while(lp_it != NULL && index <= MAX_TEXT_LINE){
        temp_print_line(lp_it, temp_position.line, wp->y_cursor, wp->position, wp->selected);
        ++temp_position.line;
        ++index;

        lp_it = lp_it->l_next;
    }

    render_statbar(wp);
    render_status_bar(wp);
}

void clear_screen(void){
    putc(ESCAPE, stdout);
    putc(CSI, stdout);
    printf("2J");
}

static void clear_line(void){
    putc(ESCAPE, stdout);
    putc(CSI, stdout);
    printf("2K");
}

void print_line(struct window* wp){
    struct line_position temp_position = {.line = wp->y_cursor, .column = wp->position.column};
    struct unicode_column* unicode_it = wp->current_lp->l_content;

    if(wp->position.line <= MAX_TEXT_LINE)
        temp_position.line = wp->position.line;
    
    move_cursor_from_line(temp_position.line, 0);
    clear_line();

    for(size_t i = 0; i < wp->current_lp->l_size; ++i){
        if(unicode_it == NULL)
            break;

        if(wp->selected.is_selected && wp->position.line == wp->selected.line && (i >= wp->selected.start && i < wp->selected.end))
            set_color(100);
        else
            reset_color();
        
        tputc(unicode_it->unicode);

        unicode_it = unicode_it->u_next;
    }

    putc('\n', stdout);
    temp_move_cursor(temp_position);
}


