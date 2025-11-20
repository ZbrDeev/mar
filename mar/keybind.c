#include "keybind.h"
#include "hashmap.h"
#include "ansi.h"
#include "line.h"
#include "utf8.h"
#include <poll.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

static bool should_close = false;

static size_t line = 0;
static size_t column = 0;

static struct line* first_lp;
static struct line* current_lp;

static struct hashmap keybind_hashmap;

// TODO: If we have a big message with some multiple utf8 for example (hel🙂o) this will produce an error because we have a max message of 4 byte

static void move_cursor_up(void){
    if(line >= 1){
        --line;
        move_cursor(line, column);        

        if(current_lp->l_back != NULL)
            current_lp = current_lp->l_back;
    }else{
        line = 0;
        current_lp = first_lp;
    }
}

static void move_cursor_down(void){
    if(current_lp->l_next != NULL){
        current_lp = current_lp->l_next;
        ++line;
        move_cursor(line, column);
    }
}

static void move_cursor_right(void){
    if(column < current_lp->l_size){
        ++column;
        move_cursor(line, column);    
    }
}

static void move_cursor_left(void){
    if(column >= 1){
        --column;
        move_cursor(line, column);
    }else{
        column = 0;
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

static void exec_function(unsigned char keys[MAX_KEY_SIZE], size_t index){
    unsigned key_maker = keys[0];

    for(size_t i = 1; i < index; ++i){
        key_maker &= keys[i];
    }

    struct node* np = h_get_value(&keybind_hashmap, key_maker);

    if(np != NULL && np->function != NULL){
        np->function();
    }
}

static void update_line(unsigned char keys[MAX_KEY_SIZE], size_t index){
    size_t i = 0;
    while(i < index){
        struct unicode_encoding unicode = utf8_to_unicode(keys, i, index);

        i += unicode.bytes_size;

        insert_char_in_line(current_lp, unicode, column);
        ++column;
    }

    erase_line();
    print_line(current_lp, line, column);
}

static void remove_char(void){
    if(column == 0 && line > 0){
        struct line* lp_temp = current_lp;
        current_lp = current_lp->l_back;

        current_lp->l_last_content->u_next = lp_temp->l_content;
        lp_temp->l_content->u_back = current_lp->l_last_content;

        current_lp->l_last_content = lp_temp->l_last_content;

        if(lp_temp->l_next != NULL){
            lp_temp->l_next->l_back = current_lp;
            current_lp->l_next = lp_temp->l_next;
        }else{
            current_lp->l_next = NULL;
        }

        --line;
        column = current_lp->l_size;
        current_lp->l_size += lp_temp->l_size;

        free(lp_temp);
        print_screen(first_lp, line, column);

        return;
    }else if(column == 0){
        return;
    }

    struct unicode_column* unicode_it = current_lp->l_content;
    
    for(size_t i = 0; i < column; ++i){
        if(unicode_it->u_next == NULL){
            break;
        }

        unicode_it = unicode_it->u_next;
    }

    struct unicode_column* temp = unicode_it->u_back;

    if(temp->u_back == NULL){
        current_lp->l_content = current_lp->l_content->u_next;
        current_lp->l_content->u_back = NULL;
        goto done;
    }
    
    unicode_it->u_back = temp->u_back;
    temp->u_back->u_next = unicode_it;
    
done:
    free(temp);
    --column;
    --current_lp->l_size;
    erase_line();
    print_line(current_lp, line, column);
}

void read_key(struct line* lp){
    first_lp = lp;
    current_lp = lp;

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
                exec_function(keys, index);
            }else if(keys[0] == DELETE_KEY){
                remove_char();
            }else{
                update_line(keys, index);
            }

            index = 0;
        }
    }

    h_free(&keybind_hashmap);
}
