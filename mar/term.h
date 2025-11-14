#ifndef TERM_H
#define TERM_H

#include "utf8.h"
#include <termios.h>
#include <stdbool.h>

#define MAX_LINE 100
#define MAX_COL 100

static struct termios newterm, oldterm;

static int line = 1;
static int column = 1;
static bool should_close = false;

void init_term(void);

void close_term(void);

void flush(void);

void tputc(struct unicode_encoding unicode);

#endif // TERM_H