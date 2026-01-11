#ifndef TERM_H
#define TERM_H

#include "utf8.h"
#include <termios.h>

void init_term(void);

void close_term(void);

void flush(void);

void tputc(struct unicode_encoding unicode);

#endif // TERM_H