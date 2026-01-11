#include "keybind.h"
#include "file.h"
#include "hashmap.h"
#include "ansi.h"
#include "line.h"
#include "system.h"
#include "utf8.h"
#include "window.h"
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

        if(current_wp->y_cursor > 0){
            --current_wp->y_cursor;
            move_cursor(current_wp);
        }
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


        if(current_wp->y_cursor < MAX_TEXT_LINE){
            ++current_wp->y_cursor;
            move_cursor(current_wp);
        }
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

    move_cursor(current_wp);
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

    move_cursor(current_wp);
}

static void move_first_column(void){
    current_wp->position.column = 0;
    move_cursor(current_wp);
}

static void move_last_column(void){
    current_wp->position.column = current_wp->current_lp->l_size;
    move_cursor(current_wp);
}

static void move_word_backward(void){
    struct unicode_column* unicode_it = current_wp->current_lp->l_content;
    
    size_t current_column = current_wp->position.column;
    for(size_t i = 0; i < current_column; ++i){
        if(unicode_it->unicode.result == 0x20)
            current_wp->position.column = i;
        else if(unicode_it == NULL)
            break;
        

        unicode_it = unicode_it->u_next;
    }

    move_cursor(current_wp);
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

    move_cursor(current_wp);
}

static void reset_position(void){
    current_wp->position.line = 0;
    current_wp->position.column = 0;
    current_wp->current_lp = current_wp->first_lp;

    move_cursor(current_wp);
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
    move_cursor(current_wp);
}

static void select_left(void){
    struct selected_text* selected = &current_wp->selected;

    if(!selected->is_selected){
        selected->end = current_wp->position.column;
        selected->is_selected = true;
        selected->line = current_wp->position.line;
        selected->last_selected = left;
    }

    move_cursor_left();

    if(selected->line != current_wp->position.line){
        selected->is_selected = false;
        return;
    }
    
    if(selected->start == selected->end){
        selected->is_selected = false;
    }else{
        if(selected->last_selected != left)
            selected->end = current_wp->position.column;
        else
            selected->start = current_wp->position.column;
        
    }
    
    print_line(current_wp);
}

static void select_right(void){
    struct selected_text* selected = &current_wp->selected;

    if(!selected->is_selected){
        selected->start = current_wp->position.column;
        selected->is_selected = true;
        selected->line = current_wp->position.line;
        selected->last_selected = right;
    }

    move_cursor_right();

    if(selected->line != current_wp->position.line){
        selected->is_selected = false;
        return;
    }
    
    if(selected->start == selected->end){
        selected->is_selected = false;
    }else{
        if(selected->last_selected != right)
            selected->start = current_wp->position.column;
        else
            selected->end = current_wp->position.column;
        
    }

    print_line(current_wp);
}

static void copy_text(void){
    if(!current_wp->selected.is_selected){
        current_wp->selected.is_selected = true;

        current_wp->selected.line = current_wp->position.line;
        current_wp->selected.start = 0;
        current_wp->selected.end = current_wp->current_lp->l_size;
    }

    size_t size = 0;
    char* selected_char = return_copied_char(current_wp->current_lp, current_wp->selected.start, current_wp->selected.end, &size);

    copy_to_clipboard(selected_char, size);

    free(selected_char);
}

static void quit_terminal(void){
    should_close = true;
}

static void save(void){
    save_file(current_wp->filename, current_wp->first_lp);
    current_wp->status_bar_text = "File saved successfully";
}


void init_keybind(void){
    keybind_hashmap = h_init();

    // Navigation control
    h_insert_value(&keybind_hashmap, ESCAPE + CSI + 0x41, &move_cursor_up);
    h_insert_value(&keybind_hashmap, ESCAPE + CSI + 0x42, &move_cursor_down);
    h_insert_value(&keybind_hashmap, ESCAPE + CSI + 0x43, &move_cursor_right);
    h_insert_value(&keybind_hashmap, ESCAPE + CSI + 0x44, &move_cursor_left);

    h_insert_value(&keybind_hashmap, ESCAPE + CSI + 0x46, &move_last_column);
    h_insert_value(&keybind_hashmap, ESCAPE + CSI + 0x48, &move_first_column);

    h_insert_value(&keybind_hashmap, ESCAPE + CSI + 0x31 + 0x3b + 0x35 + 0x43, &move_word_forward);
    h_insert_value(&keybind_hashmap, ESCAPE + CSI + 0x31 + 0x3b + 0x35 + 0x44, &move_word_backward);

    h_insert_value(&keybind_hashmap, ESCAPE + CSI + 0x31 + 0x3b + 0x35 + 0x48 , &reset_position);
    h_insert_value(&keybind_hashmap, ESCAPE + CSI + 0x31 + 0x3b + 0x35 + 0x46 , &move_end_of_line);

    // Select control
    h_insert_value(&keybind_hashmap, ESCAPE + CSI + 0x31 + 0x3b + 0x32 + 0x43, &select_right);
    h_insert_value(&keybind_hashmap, ESCAPE + CSI + 0x31 + 0x3b + 0x32 + 0x44, &select_left);

    // Terminal control
    h_insert_value(&keybind_hashmap, CTRL('q') , &quit_terminal);

    // File control
    h_insert_value(&keybind_hashmap, CTRL('s'), &save);

    // Text control
    h_insert_value(&keybind_hashmap, ESCAPE + CSI + 0x33 + 0x7e, &remove_front_char);

    h_insert_value(&keybind_hashmap, CTRL('c'), &copy_text);
}

static void check_if_still_select(unsigned key){
    if(key == ESCAPE + CSI + 0x31 + 0x3b + 0x32 + 0x43 || key == ESCAPE + CSI + 0x31 + 0x3b + 0x32 + 0x44 || key == CTRL('c'))
        return;
    
    current_wp->selected.is_selected = false;
}

static void exec_function(unsigned char* keys, size_t index){
    unsigned key_maker = 0;

    for(size_t i = 0; i < index; ++i)
        key_maker += keys[i];
    

    check_if_still_select(key_maker);
    struct node* np = h_get_value(&keybind_hashmap, key_maker);

    if(np != NULL && np->function != NULL)
        np->function();
    
    print_screen(current_wp);
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
    print_line(current_wp);
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
        print_line(current_wp);
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
    print_line(current_wp);
}

// TODO: improve this code because its a mess
static void remove_back_char(void){
    if(current_wp->position.column == 0 && current_wp->position.line > 0){
        fusion_with_previous_line();
        --current_wp->position.line;
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
    print_line(current_wp);
}

static void enter(void){
    struct unicode_column* unicode_it = current_wp->current_lp->l_content;

    for(size_t i = 0; i < current_wp->position.column; ++i){
        if(unicode_it->u_next == NULL)
            break;
        
        unicode_it = unicode_it->u_next;
    }

    struct line* new_line = malloc(sizeof(struct line));

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
    
    if(current_wp->current_lp->l_next != NULL)
        new_line->l_next->l_back = new_line;
    
    
    current_wp->current_lp->l_next = new_line;
    
    current_wp->position.column = 0;
    ++current_wp->position.line;
    current_wp->current_lp = current_wp->current_lp->l_next;
}

void read_key(struct window* wp){
    current_wp = first_wp = wp;

    struct pollfd key_poll;
    key_poll.fd = STDIN_FILENO;
    key_poll.events = POLLIN;

    unsigned char *keys = malloc(1);
    size_t index = 0;

    while(!should_close) {
        poll(&key_poll, 1, -1);

        unsigned char a;
        read(STDIN_FILENO, &a, 1);

        unsigned char* temp = realloc(keys, ++index);
        assert(temp != NULL);
        keys = temp;

        keys[index-1] = a;

        if(poll(&key_poll, 1, 1) <= 0){
            if(keys[0] == DELETE_KEY)
                remove_back_char();
            else if(keys[0] == ENTER_KEY)
                enter();
            else if(keys[0] == ESCAPE || keys[0] <= 26)
                exec_function(keys, index);
            else
                update_line(keys, index);
            

            index = 0;
            free(keys);
            keys = malloc(1);
        }
    }

    free(keys);
    h_free(&keybind_hashmap);
}
