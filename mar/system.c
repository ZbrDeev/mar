#include "system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For copy and paste to the linux clipboard
void copy_to_clipboard(char* value, size_t size){
    char* session_type = getenv("XDG_SESSION_TYPE");
    FILE* fp = NULL;

    if(strcmp(session_type, "wayland") == 0){
        fp = popen("wl-copy", "w");
    }else if(strcmp(session_type, "x11") == 0){
        fp = popen("xclip -selection clipboard", "w");
    }else{
        return;
    }

    fwrite(value, 1, size, fp);
    pclose(fp);
}