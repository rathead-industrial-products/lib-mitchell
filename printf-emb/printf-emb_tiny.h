/******************************************************************************

    (c) Copyright 2019 ee-quipment.com

    printf-emb_tiny.c - printf type functions for an embedded environment.

    int  snprintf(char *s, size_t n, const char *fmt, ...)
    int  vsnprintf(char *s, size_t n, const char *fmt, va_list ap)

    What IS supported:
        All integer conversion specifiers: d, i, o, u, x, and X.
        The unsigned char specifier: c
        The pointer specifier: p.
        The string specifier: s.
        The % escape specifier: %.
        All length modifers, although only ll has any effect.
        All flag modifiers.

    What is NOT supported:
        Floating point conversion specifiers: e, E, f, F, g, G, a, A.
        The number-of-characters specifier: n.
        Wide characters.
        Any other specifier not explicitly supported.

    No division operations (including modulo, etc) are performed.
    No c libraries are used.

    A NULL output string pointer s is allowed.

    The implementation is targeted at a 32-bit or more architectures that
    promote all variadic parameters to 32 bit ints except pointers which
    may be either 32 or 64 bits depending upon the architecture. ARM and
    IA-64 have been tested.

 *****************************************************************************/


#ifndef _printf_emb_tiny_H_
#define _printf_emb_tiny_H_

#include <stddef.h>
#include <stdarg.h>


int  snprintf(char *s, size_t n, const char *fmt, ...);
int  vsnprintf(char *s, size_t n, const char *fmt, va_list ap);


#endif  /* _printf_emb_tiny_H_ */


