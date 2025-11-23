#include "file.h"
#include "line.h"
#include "utf8.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

FILE* open_file(const char* filename){
    FILE *fp;

    // Open the file in read and write mode
    if((fp = fopen(filename, "rw")) == NULL){
        printf("Invalid file path\n");
        return NULL;
    }

    return fp;
}

unsigned char* read_file(FILE* fp, size_t *size){
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

    unsigned char *buffer = (unsigned char *)malloc(*size + 1);
    buffer[*size] = '\0';

    fread(buffer, *size, 1, fp);
    fclose(fp);

    return buffer;
}

void save_file(const char* filename, struct line* lp){
    char* content_result = (char*)malloc(1);

    size_t content_size = 0;
    struct line* lp_it = lp;

    while(lp_it != NULL){
        struct unicode_column* unicode_it = lp_it->l_content;

        for(size_t i = 0; i < lp_it->l_size; ++i){
            if(unicode_it == NULL){
                break;
            }

            struct utf8_encoding utf8 = unicode_to_utf8(unicode_it->unicode);

            content_size += utf8.bytes_size;

            char* temp = (char*)realloc(content_result, content_size);
            assert(temp != NULL);
            content_result = temp;

            for(size_t j = 0; j < utf8.bytes_size; ++j){
                size_t index = content_size - utf8.bytes_size + j;
                content_result[index] = utf8.result[j];
            }

            unicode_it = unicode_it->u_next;
        }

        ++content_size;
        char* temp = (char*)realloc(content_result, content_size);
        assert(temp != NULL);
        content_result = temp;

        content_result[content_size-1] = '\n';

        lp_it = lp_it->l_next;
    }

    ++content_size;
    char* temp = (char*)realloc(content_result, content_size);
    assert(temp != NULL);
    content_result = temp;

    content_result[content_size-1] = '\0';
    

    FILE* file = fopen(filename, "w");
    size_t res = fwrite(content_result, sizeof(char), content_size, file);

    free(content_result);
    fclose(file);
}