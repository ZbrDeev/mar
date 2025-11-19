#include "keybind.h"
#include "hashmap.h"
#include "ansi.h"
#include "line.h"
#include "utf8.h"
#include <poll.h>
#include <unistd.h>
#include <stdbool.h>

static bool should_close = false;
static size_t line = 1;
static size_t column = 1;

// TODO: If we have a big message with some multiple utf8 for example (hel🙂o) this will produce an error because we have a max message of 4 byte

static void move_cursor_up(void){
    if(line >= 1){
        --line;
        move_cursor(line, column);        
    }else{
        line = 1;
    }
}

static void move_cursor_down(void){
    ++line;
    move_cursor(line, column);   
}

static void move_cursor_right(void){
    ++column;
    move_cursor(line, column);    
}

static void move_cursor_left(void){
    if(column >= 1){
        --column;
        move_cursor(line, column);
    }else{
        column = 1;
    }
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

static void exec_function(unsigned char keys[MAX_KEY_SIZE], size_t index, struct line* lp){
    unsigned key_maker = keys[0];

    for(size_t i = 1; i < index; ++i){
        key_maker &= keys[i];
    }

    struct node* np = h_get_value(&keybind_hashmap, key_maker);

    if(np != NULL && np->function != NULL){
        np->function();
    }
}

static void update_line(unsigned char keys[MAX_KEY_SIZE], size_t index, struct line* lp){
    struct line* lp_it = lp;

    for(size_t i = 0; i < line-1; ++i){
        lp_it = lp_it->l_next;
    }

    size_t i = 0;
    while(i < index){
        struct unicode_encoding unicode = utf8_to_unicode(keys, i, index);

        i += unicode.bytes_size;

        insert_char_in_line(lp_it, unicode, column);
        ++column;
    }

    print_line(lp_it, line, column);
}

void read_key(struct line* lp){
    struct pollfd key_poll;
    key_poll.fd = STDIN_FILENO;
    key_poll.events = POLLIN;

    unsigned char keys[MAX_KEY_SIZE];
    size_t index = 0;

    while(!should_close) {
        poll(&key_poll, 1, -1);

        unsigned char a;
        read(STDIN_FILENO, &a, 1);

        keys[index++] = a;

        if(poll(&key_poll, 1, 1) <= 0 || index >= MAX_KEY_SIZE){

            if(keys[0] == ESCAPE){
                exec_function(keys, index, lp);
            }else{
                update_line(keys, index, lp);
            }

            index = 0;
        }
    }

    h_free(&keybind_hashmap);
}
