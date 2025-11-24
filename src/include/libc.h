#ifndef _LIBC_H
#define _LIBC_H

#include <stddef.h>

#if __386__
# define far
#endif

int _fsnprintf(char far * str, size_t n, const char far * fmt, ...);

#endif // _LIBC_H
