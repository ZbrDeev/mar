#include "file.h"
#include <assert.h>
#include <stdlib.h>

FILE* open_file(const char* filename){
    FILE *fp;

    // Open the file in read and write mode
    if((fp = fopen(filename, "rw")) == NULL){
        printf("Invalid file path\n");
        return NULL;
    }

    return fp;
}

char* read_file(FILE* fp, size_t *size){
    // Go to the end of file and calc the size of the file content
    if (fseek(fp, 0, SEEK_END) != 0) {
        printf("Failed to read the file\n");
        fclose(fp);
        return NULL;
    }

    *size = ftell(fp);

    if (*size < 3) {
        printf("Empty file detected\n");
        fclose(fp);
        return NULL;
    }

    rewind(fp);

    char *buffer = (char *)malloc(*size + 1);
    buffer[*size] = '\0';

    fread(buffer, *size, 1, fp);
    fclose(fp);

    return buffer;
}