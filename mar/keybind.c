#include "keybind.h"
#include "file.h"
#include "hashmap.h"
#include "ansi.h"
#include "line.h"
#include "utf8.h"
#include <assert.h>
#include <poll.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

static bool should_close = false;

static struct window* first_wp;
static struct window* current_wp;

static struct hashmap keybind_hashmap;

static void remove_front_char(void);

static void move_cursor_up(void){
    if(current_wp->position.line >= 1){
        --current_wp->position.line;

        if(current_wp->current_lp->l_back != NULL)
            current_wp->current_lp = current_wp->current_lp->l_back;

        if(current_wp->position.column > current_wp->current_lp->l_size)
            current_wp->position.column = current_wp->current_lp->l_size;

        move_cursor(current_wp->position);
    }else{
        current_wp->position.line = 0;
        current_wp->position.column = 0;
        current_wp->current_lp = current_wp->first_lp;
    }
}

static void move_cursor_down(void){
    if(current_wp->current_lp->l_next != NULL){
        current_wp->current_lp = current_wp->current_lp->l_next;
        ++current_wp->position.line;

        if(current_wp->position.column > current_wp->current_lp->l_size)
            current_wp->position.column = current_wp->current_lp->l_size;

        move_cursor(current_wp->position);
    }
}

static void move_cursor_right(void){
    if(current_wp->position.column < current_wp->current_lp->l_size){
        ++current_wp->position.column;
    }else{
        if(current_wp->current_lp->l_next != NULL){
            current_wp->position.column = 0;
            ++current_wp->position.line;
            current_wp->current_lp = current_wp->current_lp->l_next;
        }
    }

    move_cursor(current_wp->position);    
}

static void move_cursor_left(void){
    if(current_wp->position.column >= 1){
        --current_wp->position.column;
    }else{
        if(current_wp->current_lp->l_back != NULL){
            current_wp->current_lp = current_wp->current_lp->l_back;
            --current_wp->position.line;
            current_wp->position.column = current_wp->current_lp->l_size;
        }
    }

    move_cursor(current_wp->position);
}

static void move_first_column(void){
    current_wp->position.column = 0;
    move_cursor(current_wp->position);
}

static void move_last_column(void){
    current_wp->position.column = current_wp->current_lp->l_size;
    move_cursor(current_wp->position);
}

static void move_word_backward(void){
    struct unicode_column* unicode_it = current_wp->current_lp->l_content;
    
    size_t current_column = current_wp->position.column;
    for(size_t i = 0; i < current_column; ++i){
        if(unicode_it->unicode.result == 0x20){
            current_wp->position.column = i;
        }else if(unicode_it == NULL){
            break;
        }

        unicode_it = unicode_it->u_next;
    }

    move_cursor(current_wp->position);
}

static void move_word_forward(void){
    struct unicode_column* unicode_it = current_wp->current_lp->l_content;
    
    for(size_t i = 0; i < current_wp->current_lp->l_size; ++i){
        if(i > current_wp->position.column && unicode_it->unicode.result == 0x20){
            current_wp->position.column = i;
            break;
        }else if(unicode_it == NULL){
            break;
        }

        unicode_it = unicode_it->u_next;
    }

    move_cursor(current_wp->position);
}

static void reset_position(void){
    current_wp->position.line = 0;
    current_wp->position.column = 0;
    current_wp->current_lp = current_wp->first_lp;

    move_cursor(current_wp->position);
}

static void move_end_of_line(void){
    size_t index = 0;
    struct line* temp_lp = current_wp->first_lp;

    while(temp_lp->l_next != NULL){
        ++index;
        temp_lp = temp_lp->l_next;
    }

    current_wp->position.line = index;
    current_wp->position.column = temp_lp->l_size;
    current_wp->current_lp = temp_lp;
    move_cursor(current_wp->position);
}

static void quit_terminal(void){
    should_close = true;
}

static void save(void){
    save_file(current_wp->filename, current_wp->first_lp);
}


void init_keybind(void){
    keybind_hashmap = h_init();

    // Navigation control
    h_insert_value(&keybind_hashmap, ESCAPE ^ '[' ^ 'A', &move_cursor_up);
    h_insert_value(&keybind_hashmap, ESCAPE ^ '[' ^ 'B', &move_cursor_down);
    h_insert_value(&keybind_hashmap, ESCAPE ^ '[' ^ 'C', &move_cursor_right);
    h_insert_value(&keybind_hashmap, ESCAPE ^ '[' ^ 'D', &move_cursor_left);

    h_insert_value(&keybind_hashmap, ESCAPE ^ '[' ^ 'F', &move_last_column);
    h_insert_value(&keybind_hashmap, ESCAPE ^ '[' ^ 'H', &move_first_column);

    h_insert_value(&keybind_hashmap, ESCAPE ^ '[' ^ 0x31 ^ 0x3b ^ 0x35 ^ 0x44, &move_word_backward);
    h_insert_value(&keybind_hashmap, ESCAPE ^ '[' ^ 0x31 ^ 0x3b ^ 0x35 ^ 0x43, &move_word_forward);

    h_insert_value(&keybind_hashmap, ESCAPE ^ '[' ^ 0x31 ^ 0x3b ^ 0x35 ^ 0x48 , &reset_position);
    h_insert_value(&keybind_hashmap, ESCAPE ^ '[' ^ 0x31 ^ 0x3b ^ 0x35 ^ 0x46 , &move_end_of_line);

    // Terminal control
    h_insert_value(&keybind_hashmap, ESCAPE ^ 'Q' , &quit_terminal);
    h_insert_value(&keybind_hashmap, ESCAPE ^ 'q' , &quit_terminal);

    // File control
    h_insert_value(&keybind_hashmap, ESCAPE ^ 'S', &save);
    h_insert_value(&keybind_hashmap, ESCAPE ^ 's', &save);

    // Text control
    h_insert_value(&keybind_hashmap, ESCAPE ^ '[' ^ 0x33 ^ 0x7e, &remove_front_char);
}

static void exec_function(unsigned char* keys, size_t index){
    unsigned key_maker = keys[0];

    for(size_t i = 1; i < index; ++i){
        key_maker ^= keys[i];
    }

    struct node* np = h_get_value(&keybind_hashmap, key_maker);

    if(np != NULL && np->function != NULL){
        np->function();
    }
}

static void update_line(unsigned char* keys, size_t index){
    size_t i = 0;
    while(i < index){
        struct unicode_encoding unicode = utf8_to_unicode(keys, i, index);

        i += unicode.bytes_size;

        insert_char_in_line(current_wp->current_lp, unicode, current_wp->position.column);
        ++current_wp->position.column;
    }

    erase_line();
    print_line(current_wp->current_lp, current_wp->position);
}

static void fusion_with_previous_line(void){
    struct line* lp_temp = current_wp->current_lp;
    current_wp->current_lp = current_wp->current_lp->l_back;

    if(current_wp->current_lp->l_content != NULL && lp_temp->l_content != NULL){
        current_wp->current_lp->l_last_content->u_next = lp_temp->l_content;
        lp_temp->l_content->u_back = current_wp->current_lp->l_last_content;

        if(lp_temp->l_last_content != NULL)
            current_wp->current_lp->l_last_content = lp_temp->l_last_content;

        current_wp->position.column = current_wp->current_lp->l_size;
    }else if(current_wp->current_lp->l_content == NULL && lp_temp->l_content != NULL){
        current_wp->current_lp->l_last_content = lp_temp->l_last_content;
        current_wp->current_lp->l_size = lp_temp->l_size;
        current_wp->current_lp->l_content = lp_temp->l_content;

        current_wp->position.column = 0;
    }

    if(lp_temp->l_next != NULL){
        lp_temp->l_next->l_back = current_wp->current_lp;
        current_wp->current_lp->l_next = lp_temp->l_next;
    }else {
        current_wp->current_lp->l_next = NULL;
    }

    current_wp->current_lp->l_size += lp_temp->l_size;

    free(lp_temp);
}


static void remove_front_char(void){
    if(current_wp->position.column == current_wp->current_lp->l_size && current_wp->current_lp->l_next != NULL){
        current_wp->current_lp = current_wp->current_lp->l_next;
        fusion_with_previous_line();
        print_screen(current_wp->first_lp, current_wp->position);
        return;
    }else if(current_wp->position.column == current_wp->current_lp->l_size){
        return;
    }

    struct unicode_column* unicode_it = current_wp->current_lp->l_content;

    for(size_t i = 0; i < current_wp->position.column; ++i){
        if(unicode_it->u_next != NULL)
            break;
        

        unicode_it = unicode_it->u_next;
    }

    struct unicode_column* temp = unicode_it->u_next;
    
    if(unicode_it->u_back == NULL){
        temp = unicode_it;
        current_wp->current_lp->l_content = unicode_it->u_next;

        if(current_wp->current_lp->l_content != NULL)
            current_wp->current_lp->l_content->u_back = NULL;

        goto done;
    }

    unicode_it->u_next = temp->u_next;
    if(temp->u_next != NULL)
        temp->u_back = unicode_it;

done:
    free(temp);
    --current_wp->current_lp->l_size;
    erase_line();
    print_line(current_wp->current_lp, current_wp->position);
}

// TODO: improve this code because its a mess
static void remove_back_char(void){
    if(current_wp->position.column == 0 && current_wp->position.line > 0){
        fusion_with_previous_line();
        --current_wp->position.line;
        print_screen(current_wp->first_lp, current_wp->position);
        return;
    }else if(current_wp->position.column == 0){
        return;
    }

    struct unicode_column* unicode_it = current_wp->current_lp->l_content;
    
    for(size_t i = 0; i < current_wp->position.column; ++i){
        if(unicode_it->u_next == NULL)
            break;
        

        unicode_it = unicode_it->u_next;
    }

    struct unicode_column* temp = unicode_it->u_back;
    
    if(unicode_it->u_next == NULL){
        if(unicode_it->u_back != NULL)
            unicode_it = unicode_it->u_back;
        
        unicode_it->u_next = NULL;

        temp = unicode_it->u_next;
        goto done;
    }
    
    if(temp == NULL){
        current_wp->current_lp->l_content = NULL;
        current_wp->current_lp->l_last_content = NULL;
        current_wp->current_lp->l_size = 0;
        free(unicode_it);

        current_wp->position.column = 0;
        erase_line();
        print_line(current_wp->current_lp, current_wp->position);

        return;
    }

    if(temp->u_back == NULL){
        current_wp->current_lp->l_content = current_wp->current_lp->l_content->u_next;
        current_wp->current_lp->l_content->u_back = NULL;
        goto done;
    }
    
    unicode_it->u_back = temp->u_back;
    temp->u_back->u_next = unicode_it;
    
done:
    free(temp);
    --current_wp->position.column;
    --current_wp->current_lp->l_size;
    erase_line();
    print_line(current_wp->current_lp, current_wp->position);
}

static void enter(void){
    struct unicode_column* unicode_it = current_wp->current_lp->l_content;

    for(size_t i = 0; i < current_wp->position.column; ++i){
        if(unicode_it->u_next == NULL)
            break;
        

        unicode_it = unicode_it->u_next;
    }

    struct line* new_line = (struct line*)malloc(sizeof(struct line));

    if(unicode_it == NULL || unicode_it->u_next == NULL){
        new_line->l_content = NULL;
        new_line->l_size = 0;
        new_line->l_last_content = NULL;
    }else{
        new_line->l_content = unicode_it;
        new_line->l_size = current_wp->current_lp->l_size - current_wp->position.column;
        new_line->l_last_content = current_wp->current_lp->l_last_content;

        if(unicode_it->u_back != NULL){
            current_wp->current_lp->l_last_content = unicode_it->u_back;
            current_wp->current_lp->l_last_content->u_next = NULL;
            current_wp->current_lp->l_size = current_wp->position.column;
        }else{
            current_wp->current_lp->l_content = NULL;
            current_wp->current_lp->l_last_content = NULL;
            current_wp->current_lp->l_size = 0;
        }

        new_line->l_content->u_back = NULL;
    }

    new_line->l_back = current_wp->current_lp;
    new_line->l_next = current_wp->current_lp->l_next;
    
    if(current_wp->current_lp->l_next != NULL){
        new_line->l_next->l_back = new_line;
    }
    
    current_wp->current_lp->l_next = new_line;
    
    current_wp->position.column = 0;
    ++current_wp->position.line;
    current_wp->current_lp = current_wp->current_lp->l_next;

    print_screen(current_wp->first_lp, current_wp->position);
}

void read_key(struct window* wp){
    current_wp = first_wp = wp;

    struct pollfd key_poll;
    key_poll.fd = STDIN_FILENO;
    key_poll.events = POLLIN;

    unsigned char *keys = (unsigned char*)malloc(1);
    size_t index = 0;

    while(!should_close) {
        poll(&key_poll, 1, -1);

        unsigned char a;
        read(STDIN_FILENO, &a, 1);

        unsigned char* temp = (unsigned char*)realloc(keys, ++index);
        assert(temp != NULL);
        keys = temp;

        keys[index-1] = a;

        if(poll(&key_poll, 1, 1) <= 0){

            if(keys[0] == ESCAPE){
                exec_function(keys, index);
            }else if(keys[0] == DELETE_KEY){
                remove_back_char();
            }else if(keys[0] == ENTER_KEY){
                enter();
            }else{
                update_line(keys, index);
            }

            index = 0;
            free(keys);
            keys = (unsigned char*)malloc(1);
        }
    }

    free(keys);
    h_free(&keybind_hashmap);
}
