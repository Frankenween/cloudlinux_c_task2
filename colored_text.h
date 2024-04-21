#pragma once

#include <stdio.h>

enum ansi_color {
    RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN
};

void cvprintf(enum ansi_color color, const char *fmt, va_list args);

void cprintf(enum ansi_color color, const char *fmt, ...);
