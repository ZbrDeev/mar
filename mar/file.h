#ifndef FILE_H
#define FILE_H

#include "stdio.h"

FILE* open_file(const char* filename);

unsigned char* read_file(FILE* fp, size_t *size);

#endif // FILE_H