#include "keybind.h"
#include "hashmap.h"
#include "ansi.h"
#include "term.h"
#include <poll.h>
#include <unistd.h>

static void move_cursor_up(void){
    if(line > 1){
        --line;
        move_cursor();
        erase_line();
    }
}

static void move_cursor_down(void){
    ++line;
    move_cursor();
    erase_line();
}

static void move_cursor_right(void){
    if(column > 1){
        --column;
    }

    move_cursor();
    erase_line();
}

static void move_cursor_left(void){
    ++column;
    move_cursor();
    erase_line();
}

static void quit_terminal(void){
    should_close = true;
}


void init_keybind(void){
    keybind_hashmap = h_init();

    // Navigation control
    h_insert_value(&keybind_hashmap, ESCAPE & '[' & 'A', &move_cursor_up);
    h_insert_value(&keybind_hashmap, ESCAPE & '[' & 'B', &move_cursor_down);
    h_insert_value(&keybind_hashmap, ESCAPE & '[' & 'C', &move_cursor_right);
    h_insert_value(&keybind_hashmap, ESCAPE & '[' & 'D', &move_cursor_left);

    // Terminal control
    h_insert_value(&keybind_hashmap, ESCAPE & 'Q' , &quit_terminal);
}

static void exec_function(unsigned char keys[3], unsigned index){
    unsigned key_maker = keys[0];

    for(size_t i = 1; i<index; ++i){
        key_maker &= keys[i];
    }

    struct node* np = h_get_value(&keybind_hashmap, key_maker);

    if(np != NULL && np->function != NULL){
        np->function();
    }
}

void read_key(){
    struct pollfd key_poll;
    key_poll.fd = STDIN_FILENO;
    key_poll.events = POLLIN;

    unsigned char keys[3];
    size_t index = 0;

    while(!should_close) {
        poll(&key_poll, 1, -1);

        unsigned char a;
        read(STDIN_FILENO, &a, 1);

        keys[index++] = a;

        if(poll(&key_poll, 1, 1) <= 0 || index >= 3){
            exec_function(keys, index);
            index = 0;
        }
    }

    h_free(&keybind_hashmap);
}
