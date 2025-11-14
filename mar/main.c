#include "ansi.h"
#include "term.h"
#include <stdlib.h>

int main(int argc, __attribute__((unused)) const char *argv[]) {
    if(argc < 1){
        return EXIT_FAILURE;
    };

    clear_screen();

    init_term();

    close_term();
    return EXIT_SUCCESS;
}