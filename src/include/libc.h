#ifndef _LIBC_H
#define _LIBC_H

/** Common routines needed for development **/

#include <stddef.h>

#if __386__
# define far
#endif

/* Watcom's sprintf does not work under Windows 1.0-3.0, only 3.1 */
/** Prints a formatted string to a (far) pointer, writing at most the specified number of bytes **/
int _fsnprintf(char far * str, size_t n, const char far * fmt, ...);

#endif // _LIBC_H
