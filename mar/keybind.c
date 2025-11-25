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

static size_t line = 0;
static size_t column = 0;

static struct line* first_lp;
static struct line* current_lp;

static struct hashmap keybind_hashmap;
static char* filename;

static void remove_front_char(void);

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
    }else{
        if(current_lp->l_next != NULL){
            column = 0;
            ++line;
            current_lp = current_lp->l_next;
        }
    }

    move_cursor(line, column);    
}

static void move_cursor_left(void){
    if(column >= 1){
        --column;
    }else{
        if(current_lp->l_back != NULL){
            current_lp = current_lp->l_back;
            --line;
            column = current_lp->l_size;
        }
    }

    move_cursor(line, column);
}

static void move_first_column(void){
    column = 0;
    move_cursor(line, column);
}

static void move_last_column(void){
    column = current_lp->l_size;
    move_cursor(line, column);
}

static void quit_terminal(void){
    should_close = true;
}

static void save(void){
    save_file(filename, first_lp);
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

        insert_char_in_line(current_lp, unicode, column);
        ++column;
    }

    erase_line();
    print_line(current_lp, line, column);
}

static void fusion_with_previous_line(void){
    struct line* lp_temp = current_lp;
    current_lp = current_lp->l_back;

    if(current_lp->l_content != NULL && lp_temp->l_content != NULL){
        current_lp->l_last_content->u_next = lp_temp->l_content;
        lp_temp->l_content->u_back = current_lp->l_last_content;

        if(lp_temp->l_last_content != NULL)
            current_lp->l_last_content = lp_temp->l_last_content;

        column = current_lp->l_size;
    }else if(current_lp->l_content == NULL && lp_temp->l_content != NULL){
        current_lp->l_last_content = lp_temp->l_last_content;
        current_lp->l_size = lp_temp->l_size;
        current_lp->l_content = lp_temp->l_content;

        column = 0;
    }

    if(lp_temp->l_next != NULL){
        lp_temp->l_next->l_back = current_lp;
        current_lp->l_next = lp_temp->l_next;
    }else {
        current_lp->l_next = NULL;
    }

    current_lp->l_size += lp_temp->l_size;

    free(lp_temp);
}


static void remove_front_char(void){
    if(column == current_lp->l_size && current_lp->l_next != NULL){
        current_lp = current_lp->l_next;
        fusion_with_previous_line();
        print_screen(first_lp, line, column);
        return;
    }else if(column == current_lp->l_size){
        return;
    }

    struct unicode_column* unicode_it = current_lp->l_content;

    for(size_t i = 0; i < column; ++i){
        if(unicode_it->u_next != NULL)
            break;
        

        unicode_it = unicode_it->u_next;
    }

    struct unicode_column* temp = unicode_it->u_next;
    
    if(unicode_it->u_back == NULL){
        temp = unicode_it;
        current_lp->l_content = unicode_it->u_next;

        if(current_lp->l_content != NULL)
            current_lp->l_content->u_back = NULL;

        goto done;
    }

    unicode_it->u_next = temp->u_next;
    if(temp->u_next != NULL)
        temp->u_back = unicode_it;

done:
    free(temp);
    --current_lp->l_size;
    erase_line();
    print_line(current_lp, line, column);
}

// TODO: improve this code because its a mess
static void remove_back_char(void){
    if(column == 0 && line > 0){
        fusion_with_previous_line();
        --line;
        print_screen(first_lp, line, column);
        return;
    }else if(column == 0){
        return;
    }

    struct unicode_column* unicode_it = current_lp->l_content;
    
    for(size_t i = 0; i < column; ++i){
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
        current_lp->l_content = NULL;
        current_lp->l_last_content = NULL;
        current_lp->l_size = 0;
        free(unicode_it);

        column = 0;
        erase_line();
        print_line(current_lp, line, column);

        return;
    }

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

static void enter(void){
    struct unicode_column* unicode_it = current_lp->l_content;

    for(size_t i = 0; i < column; ++i){
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
        new_line->l_size = current_lp->l_size - column;
        new_line->l_last_content = current_lp->l_last_content;

        if(unicode_it->u_back != NULL){
            current_lp->l_last_content = unicode_it->u_back;
            current_lp->l_last_content->u_next = NULL;
            current_lp->l_size = column;
        }else{
            current_lp->l_content = NULL;
            current_lp->l_last_content = NULL;
            current_lp->l_size = 0;
        }

        new_line->l_content->u_back = NULL;
    }

    new_line->l_back = current_lp;
    new_line->l_next = current_lp->l_next;
    
    if(current_lp->l_next != NULL){
        new_line->l_next->l_back = new_line;
    }
    
    current_lp->l_next = new_line;
    
    column = 0;
    ++line;
    current_lp = current_lp->l_next;

    print_screen(first_lp, line, column);
}

void read_key(struct line* lp, char* fp){
    first_lp = lp;
    current_lp = lp;
    filename = fp;

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
