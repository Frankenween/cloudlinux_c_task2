#include <stdarg.h>
#include "colored_text.h"

#define ANSI_RESET_COLORS  "\x1B[0m"

static const char* color_sequences[] = {
    [RED]     = "\x1b[31m",
    [GREEN]   = "\x1b[32m",
    [YELLOW]  = "\x1b[33m",
    [BLUE]    = "\x1b[34m",
    [MAGENTA] = "\x1b[35m",
    [CYAN]    = "\x1b[36m"
};

void cvprintf(enum ansi_color color, const char *fmt, va_list args) {
    printf("%s", color_sequences[color]);
    vprintf(fmt, args);
    printf(ANSI_RESET_COLORS);
}

void cprintf(enum ansi_color color, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    cvprintf(color, fmt, args);
    va_end(args);
}