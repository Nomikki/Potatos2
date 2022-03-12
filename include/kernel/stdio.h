#ifndef __STDIO_H__
#define __STDIO_H__

#include <stdint.h>

#define NULL 0
#define EOF -1

extern int32_t printf(const char *format, ...);
extern int32_t putchar(int32_t character);
extern int32_t puts(const char *str);

#endif // __STDIO_H__