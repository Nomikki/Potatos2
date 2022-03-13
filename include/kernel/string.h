

#ifndef __STRING_H__
#define __STRING_H__

#include <stdint.h>

extern size_t strlen(const char *str);
extern void *memcpy(void *dest, const void *src, size_t n);
extern void *memset(void *s, int c, size_t n);


#endif // __STRING_H__