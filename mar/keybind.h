#ifndef KEYBIND_H
#define KEYBIND_H

#include "hashmap.h"
#include "line.h"
#include "window.h"

#define UP ESCAPE + CSI + 0x41
#define DOWN ESCAPE + CSI + 0x42
#define RIGHT ESCAPE + CSI + 0x43
#define LEFT ESCAPE + CSI + 0x44

#define FIN ESCAPE + CSI + 0x46
#define ORIG ESCAPE + CSI + 0x48

#define PG_BACK ESCAPE + CSI + 0x35 + 0x7e
#define PG_NEXT ESCAPE + CSI + 0x36 + 0x7e

#define CTRL_RIGHT ESCAPE + CSI + 0x31 + 0x3b + 0x35 + 0x43
#define CTRL_LEFT ESCAPE + CSI + 0x31 + 0x3b + 0x35 + 0x44

#define CTRL_ORIG ESCAPE + CSI + 0x31 + 0x3b + 0x35 + 0x48
#define CTRL_FIN ESCAPE + CSI + 0x31 + 0x3b + 0x35 + 0x46 

#define SHIFT_RIGHT ESCAPE + CSI + 0x31 + 0x3b + 0x32 + 0x43
#define SHIFT_LEFT ESCAPE + CSI + 0x31 + 0x3b + 0x32 + 0x44

#define SUPPR ESCAPE + CSI + 0x33 + 0x7e

#define SPACE 0x20

void init_keybind(void);

void read_key(struct window* wp);

#endif // KEYBIND_H
