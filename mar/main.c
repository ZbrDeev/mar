#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if(argc < 1){
        return EXIT_FAILURE;
    };

    printf("%s\n", argv[0]);

    return EXIT_SUCCESS;
}