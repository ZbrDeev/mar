#ifndef FILE_H
#define FILE_H

#include "line.h"
#include <stdio.h>

FILE* open_file(const char* filename);

unsigned char* read_file(FILE* fp, size_t *size);

void save_file(const char* filename, struct line* lp);

#endif // FILE_H