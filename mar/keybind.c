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

// If true we should end the process
static bool should_close = false;

static struct window* first_wp;
static struct window* current_wp;

// This is a hashmap for binding function to touch
static struct hashmap keybind_hashmap;

// Remove the front char when we press SUPPR touch
static void remove_front_char(void);

static void move_cursor_up(void){
    if(current_wp->position.line == 0)
        return;

    current_wp->current_lp = current_wp->current_lp->l_back;
    --current_wp->position.line;

    if(current_wp->position.column > current_wp->current_lp->l_size)
        current_wp->position.column = current_wp->current_lp->l_size;

    if(current_wp->y_cursor > 0)
        --current_wp->y_cursor;

    if(current_wp->position.line == MAX_TEXT_LINE)
        current_wp->y_cursor = current_wp->position.line;

    move_cursor(current_wp);
}

static void move_cursor_down(void){
    if(current_wp->current_lp->l_next == NULL)
        return;

    current_wp->current_lp = current_wp->current_lp->l_next;
    ++current_wp->position.line;

    if(current_wp->position.column > current_wp->current_lp->l_size)
        current_wp->position.column = current_wp->current_lp->l_size;

    if(current_wp->y_cursor < MAX_TEXT_LINE)
        ++current_wp->y_cursor;

    move_cursor(current_wp);
}

static void move_cursor_right(void){
    if(current_wp->position.column < current_wp->current_lp->l_size){
        ++current_wp->position.column;
    }else if(current_wp->current_lp->l_next != NULL){
        current_wp->position.column = 0;
        ++current_wp->position.line;
        current_wp->current_lp = current_wp->current_lp->l_next;
    }

    move_cursor(current_wp);
}

static void move_cursor_left(void){
    if(current_wp->position.column >= 1){
        --current_wp->position.column;
    }else if(current_wp->current_lp->l_back != NULL){
        current_wp->current_lp = current_wp->current_lp->l_back;
        --current_wp->position.line;
        current_wp->position.column = current_wp->current_lp->l_size;
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

    // Here we check every space and take the last space we found
    for(size_t i = 0; i < current_column; ++i){
        if(unicode_it == NULL)
            break;

        if(unicode_it->unicode.result == SPACE)
            current_wp->position.column = i;

        unicode_it = unicode_it->u_next;
    }

    move_cursor(current_wp);
}

static void move_word_forward(void){
    struct unicode_column* unicode_it = current_wp->current_lp->l_content;
    
    // Here we check the first space and add to the position
    for(size_t i = 0; i < current_wp->current_lp->l_size; ++i){
        if(unicode_it == NULL)
            break;

        if(i > current_wp->position.column && unicode_it->unicode.result == SPACE){
            current_wp->position.column = i;
            break;
        }

        unicode_it = unicode_it->u_next;
    }

    move_cursor(current_wp);
}

static void reset_position(void){
    current_wp->position.line = 0;
    current_wp->position.column = 0;
    current_wp->y_cursor = 0;
    current_wp->current_lp = current_wp->first_lp;

    move_cursor(current_wp);
}

static void move_end_of_line(void){
    struct line* temp_lp = current_wp->current_lp;

    while(temp_lp->l_next != NULL){
        ++current_wp->position.line;
        temp_lp = temp_lp->l_next;
    }

    current_wp->position.column = temp_lp->l_size;
    current_wp->current_lp = temp_lp;
    current_wp->y_cursor = MAX_TEXT_LINE;
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
        if(selected->last_selected == left)
            selected->start = current_wp->position.column;
        else
            selected->end = current_wp->position.column;
    }
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
        if(selected->last_selected == right)
            selected->end = current_wp->position.column;
        else
            selected->start = current_wp->position.column;
    }
}

static void copy_text(void){
    size_t size = 0;
    char* selected_char = return_copied_char(current_wp->current_lp, current_wp->selected.start, current_wp->selected.end, &size);

    if(!current_wp->selected.is_selected){
        current_wp->selected.is_selected = true;
        current_wp->selected.line = current_wp->position.line;
        current_wp->selected.start = 0;
        current_wp->selected.end = current_wp->current_lp->l_size;
    }

    copy_to_clipboard(selected_char, size);

    free(selected_char);
    current_wp->status_bar_text = "Text copied successfully";
}

static void quit_terminal(void){
    should_close = true;
}

static void save(void){
    save_file(current_wp->filename, current_wp->first_lp);
    current_wp->status_bar_text = "File saved successfully";
}

static void move_back_page(void){
    size_t current_line_position = current_wp->position.line;

    if(current_line_position <= 30)
        current_wp->position.line = 0;
    else
        current_wp->position.line -= 30;

    current_wp->y_cursor = 0;

    size_t i = 0;

    while(i++ < 30 && current_wp->current_lp->l_back != NULL)
        current_wp->current_lp = current_wp->current_lp->l_back;

    current_wp->position.column = 0;
}

static void move_front_page(void){
    size_t current_line_position = current_wp->position.line;
    size_t line_size = current_wp->line_size;

    if((current_line_position += 30) > line_size)
        current_wp->position.line = line_size;
    else
        current_wp->position.line = current_line_position;

    current_wp->y_cursor = 0;

    size_t i = 0;
    
    while(i++ < 30 && current_wp->current_lp->l_next != NULL)
        current_wp->current_lp = current_wp->current_lp->l_next;

    current_wp->position.column = 0;
}

void init_keybind(void){
    keybind_hashmap = h_init();

    // Navigation control
    h_insert_value(&keybind_hashmap, UP, &move_cursor_up);
    h_insert_value(&keybind_hashmap, DOWN, &move_cursor_down);
    h_insert_value(&keybind_hashmap, RIGHT, &move_cursor_right);
    h_insert_value(&keybind_hashmap, LEFT, &move_cursor_left);
    
    h_insert_value(&keybind_hashmap, FIN, &move_last_column);
    h_insert_value(&keybind_hashmap, ORIG, &move_first_column);
    
    h_insert_value(&keybind_hashmap, CTRL_RIGHT, &move_word_forward);
    h_insert_value(&keybind_hashmap, CTRL_LEFT, &move_word_backward);

    h_insert_value(&keybind_hashmap, CTRL_ORIG, &reset_position);
    h_insert_value(&keybind_hashmap, CTRL_FIN, &move_end_of_line);

    h_insert_value(&keybind_hashmap, PG_BACK, &move_back_page);
    h_insert_value(&keybind_hashmap, PG_NEXT, &move_front_page);

    // Select control
    h_insert_value(&keybind_hashmap, SHIFT_RIGHT, &select_right);
    h_insert_value(&keybind_hashmap, SHIFT_LEFT, &select_left);

    // Terminal control
    h_insert_value(&keybind_hashmap, CTRL('q') , &quit_terminal);

    // File control
    h_insert_value(&keybind_hashmap, CTRL('s'), &save);

    // Text control
    h_insert_value(&keybind_hashmap, SUPPR, &remove_front_char);

    h_insert_value(&keybind_hashmap, CTRL('c'), &copy_text);
}

// Here we check if the the key is different than left/right shift and CTRL + C
// If it's different we update the is_selected value to false
static void check_if_still_select(unsigned key){
    if(key == SHIFT_RIGHT || key == SHIFT_LEFT || key == CTRL('c'))
        return;
    
    current_wp->selected.is_selected = false;
}

// Execute only key function event
static void exec_function(unsigned char* keys, size_t index){
    unsigned key_maker = 0;

    // We calculate the key for the hashmap
    for(size_t i = 0; i < index; ++i)
        key_maker += keys[i];
    

    check_if_still_select(key_maker);
    struct node* np = h_get_value(&keybind_hashmap, key_maker);

    if(np != NULL && np->function != NULL)
        np->function();
    
    print_screen(current_wp);
}

// Update line by adding the user character entered
static void update_line(unsigned char* keys, size_t index){
    size_t i = 0;

    while(i < index){
        struct unicode_encoding unicode = utf8_to_unicode(keys, i, index);

        i += unicode.bytes_size;

        insert_char_in_line(current_wp->current_lp, unicode, current_wp->position.column);
        current_wp->position.column += unicode.result == 0x9 ? 2 : 1;
    }

    print_line(current_wp);
}

static void fusion_with_previous_line(void){
    struct line* lp_temp = current_wp->current_lp;
    current_wp->current_lp = current_wp->current_lp->l_back;

    if(current_wp->current_lp->l_content != NULL && lp_temp->l_content != NULL){
        // If both lines are not empty fusion them

        struct unicode_column* last_content = current_wp->current_lp->l_last_content;
        struct unicode_column* temp_content = lp_temp->l_content;

        last_content->u_next = temp_content;
        temp_content->u_back = last_content;

        if(lp_temp->l_last_content != NULL)
            current_wp->current_lp->l_last_content = lp_temp->l_last_content;

        current_wp->position.column = current_wp->current_lp->l_size;
    }else if(current_wp->current_lp->l_content == NULL && lp_temp->l_content != NULL){
        // If the current line is empty just take the temp line

        current_wp->current_lp->l_last_content = lp_temp->l_last_content;
        current_wp->current_lp->l_content = lp_temp->l_content;

        current_wp->position.column = 0;
    }

    if(lp_temp->l_next != NULL){
        struct line* next_line = lp_temp->l_next;

        next_line->l_back = current_wp->current_lp;
        current_wp->current_lp->l_next = next_line;
    }else {
        current_wp->current_lp->l_next = NULL;
    }

    current_wp->position.column = current_wp->current_lp->l_size;
    current_wp->current_lp->l_size += lp_temp->l_size;

    free(lp_temp);
    --current_wp->line_size;
}


// Remove the front char when we press SUPPR touch
static void remove_front_char(void){
    // Here we check if we are at the end of the line,
    // if so do a fusion with the next line if it's not empty
    if(current_wp->position.column == current_wp->current_lp->l_size){
        if(current_wp->current_lp->l_next != NULL){
            current_wp->current_lp = current_wp->current_lp->l_next;
            fusion_with_previous_line();
            print_line(current_wp);
        }

        return;
    }

    struct unicode_column* unicode_it = current_wp->current_lp->l_content;
    size_t i = 0;

    while(i++ < current_wp->position.column && unicode_it->u_next != NULL)
        unicode_it = unicode_it->u_next;

    struct unicode_column* temp_front_unicode = unicode_it->u_next;

    // If the unicode_it is the first character in the line
    if(unicode_it->u_back == NULL){
        if(temp_front_unicode != NULL)
          temp_front_unicode->u_back = NULL;

        current_wp->current_lp->l_content = temp_front_unicode;
    }else{
        struct unicode_column* temp_back_unicode = unicode_it->u_back;
        
        temp_back_unicode->u_next = temp_front_unicode;

        if(temp_front_unicode != NULL)
          temp_front_unicode->u_back = temp_back_unicode;
        else
          current_wp->current_lp->l_last_content = temp_back_unicode;
    }

    free(unicode_it);
    --current_wp->current_lp->l_size;
    print_line(current_wp);
}

static void remove_selected_char(void){
    size_t selected_char_count = current_wp->selected.end - current_wp->selected.start;
    struct unicode_column* unicode_it = current_wp->current_lp->l_content;
    struct unicode_column* unicode_before_selected = NULL;
    size_t i = 0;

    while(i++ < current_wp->selected.start && unicode_it->u_next != NULL)
        unicode_it = unicode_it->u_next;

    unicode_before_selected = unicode_it->u_back;

    // Remove the selected character
    for(size_t i = 0; i < selected_char_count; ++i){
        if(unicode_it == NULL)
            break;

        struct unicode_column* temp = unicode_it;
        unicode_it = unicode_it->u_next;

        free(temp);
        temp = NULL;
    }

    if(unicode_before_selected == NULL){
        current_wp->current_lp->l_content = unicode_it;
    }else{
        unicode_before_selected->u_next = unicode_it;
        unicode_it->u_back = unicode_before_selected;
    }

    current_wp->current_lp->l_size -= selected_char_count;
    current_wp->position.column = current_wp->selected.start;
    current_wp->selected.is_selected = false;
}

// Remove the back char when we press DELETE key
static void remove_back_char(void){
    if(current_wp->selected.is_selected){
        remove_selected_char();
        print_screen(current_wp);

        return;
    }

    if(current_wp->position.column == 0){
        if(current_wp->position.line > 0){
            fusion_with_previous_line();
            --current_wp->position.line;
            print_screen(current_wp);
        }

        return;
    }

    struct unicode_column* unicode_it = current_wp->current_lp->l_content;
    size_t i = 0;
    
    while(i++ < current_wp->position.column && unicode_it->u_next != NULL)
        unicode_it = unicode_it->u_next;

    struct unicode_column* temp = unicode_it->u_back;
    
    // TODO: wtf ?
    if(temp == NULL){
        current_wp->current_lp->l_content = NULL;
        current_wp->current_lp->l_last_content = NULL;
        current_wp->current_lp->l_size = 0;
        
        free(unicode_it);

        current_wp->position.column = 0;
        print_line(current_wp);

        return;
    }
    
    // If the deleted character is the last in the line just free it 
    if(unicode_it->u_next == NULL && current_wp->current_lp->l_size > 1 && current_wp->position.column == current_wp->current_lp->l_size - 1){
        unicode_it->u_back = temp->u_back;
        
        if(unicode_it->u_back != NULL)
            unicode_it->u_back->u_next = unicode_it;
        else
            current_wp->current_lp->l_content = unicode_it;

        goto done;
    }else if(unicode_it->u_next == NULL){
        struct unicode_column* temp_current_unicode = unicode_it;

        if(temp != NULL)
            unicode_it = temp;
        
        unicode_it->u_next = NULL;
        temp = temp_current_unicode;

        goto done;
    }

    if(temp->u_back == NULL){
        current_wp->current_lp->l_content = current_wp->current_lp->l_content->u_next;
        current_wp->current_lp->l_content->u_back = NULL;
    }else{
        unicode_it->u_back = temp->u_back;
        temp->u_back->u_next = unicode_it;
    }
    
done:
    free(temp);
    --current_wp->position.column;
    --current_wp->current_lp->l_size;
    print_line(current_wp);
}

static void enter(void){
    struct unicode_column* unicode_it = current_wp->current_lp->l_content;
    size_t i = 0;

    while(i++ < current_wp->position.column && unicode_it != NULL)
        unicode_it = unicode_it->u_next;

    struct line* new_line = malloc(sizeof(struct line));
    assert(new_line != NULL);

    if(unicode_it == NULL){
        // If the current unicode or the next one is null
        // Just create a new empty line

        new_line->l_content = NULL;
        new_line->l_size = 0;
        new_line->l_last_content = NULL;
    }else if(unicode_it->u_next == NULL && current_wp->current_lp->l_size > 1){
        new_line->l_last_content = new_line->l_content = unicode_it;
        new_line->l_size = 1;

        current_wp->current_lp->l_last_content = unicode_it->u_back;
        --current_wp->current_lp->l_size;
        
        if(current_wp->current_lp->l_last_content != NULL)
            current_wp->current_lp->l_last_content->u_next = NULL;

        unicode_it->u_back = NULL;
    }else{
        // If the current unicode is not empty
        // Create a new line as content, the current unicode

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
    
    if(new_line->l_next != NULL)
        new_line->l_next->l_back = new_line;
    
    current_wp->current_lp->l_next = new_line;
    
    current_wp->position.column = 0;
    ++current_wp->position.line;
    ++current_wp->line_size;
    current_wp->current_lp = current_wp->current_lp->l_next;

    print_screen(current_wp);
}

void read_key(struct window* wp){
    current_wp = first_wp = wp;

    struct pollfd key_poll;
    key_poll.fd = STDIN_FILENO;
    key_poll.events = POLLIN;

    unsigned char *keys = malloc(1);
    assert(keys != NULL);
    size_t index = 0;

    while(!should_close) {
        poll(&key_poll, 1, -1);

        unsigned char c;

        // We are assuming that when we read the STDIN file we have an error
        // So we should free and end the process
        if(read(STDIN_FILENO, &c, 1) != 1)
            goto end_of_loop;

        unsigned char* temp = realloc(keys, ++index);
        assert(temp != NULL);
        keys = temp;

        keys[index-1] = c;

        if(poll(&key_poll, 1, 1) <= 0){
            if(keys[0] == DELETE_KEY)
                remove_back_char();
            else if(keys[0] == ENTER_KEY)
                enter();
            else if((keys[0] == ESCAPE || keys[0] <= 26) && keys[0] != TAB)
                exec_function(keys, index);
            else
                update_line(keys, index);

            index = 0;
            free(keys);
            keys = malloc(1);
            assert(keys != NULL);
        }
    }

end_of_loop:
    free(keys);
    h_free(&keybind_hashmap);
}
