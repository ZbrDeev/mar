#include "term.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

void init_term(void){
    tcgetattr(STDIN_FILENO, &oldterm);

    newterm = oldterm;

    newterm.c_iflag &= ~(IGNBRK | BRKINT | INPCK | IGNPAR | ICRNL | INPCK | IGNCR);
    newterm.c_oflag &= ~(OPOST | OLCUC | ONLCR | OCRNL | ONLRET | OFILL);
    newterm.c_lflag &= ~(ICANON | ECHO | ECHOK | ECHOE | ECHONL | NOFLSH | ISIG);

    newterm.c_cc[VMIN] = 1;
    newterm.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &newterm);
}

void close_term(void){
    tcsetattr(STDIN_FILENO, TCSANOW, &oldterm);
}

void flush(void){
    fflush(stdout);
}

