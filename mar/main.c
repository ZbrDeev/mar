#include "ansi.h"
#include "file.h"
#include <unistd.h>
#include "keybind.h"
#include "line.h"
#include "term.h"
#include "window.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if(argc < 1 ){
        printf("Usage: mar <file>\n");
        return EXIT_FAILURE;
    }

    // Init the terminal and the keybind event
    init_term();
    init_keybind();

    FILE* fp = open_file(argv[1]);

    if(fp == NULL){
        printf("Failed to open file\n");
        close_term();
        return EXIT_FAILURE;
    }

    // Read file content
    size_t size = 0;
    unsigned char* content = read_file(fp, &size);

    if(content == NULL){
        close_term();
        return EXIT_FAILURE;
    }

    // Initiate the first window
    struct window* wp = w_alloc();
    wp->current_lp = wp->first_lp = content_to_line(content, size);
    wp->filename = argv[1];

    render_window(wp);

    // Run the keybind listener
    read_key(wp);
    
    // When the user close the editor we should free everything and reset the terminal
    free(content);
    free_window(wp);

    struct line_position position = {.line = 0, .column = 0};
    temp_move_cursor(position);
    
    clear_screen();
    close_term();
    
    return EXIT_SUCCESS;
}