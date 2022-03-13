#include <string.h>

size_t strlen(const char *str)
{
  size_t len = 0;
  while (str[len])
    len++;
  return len;
}

void *memcpy(void *d, const void *s, size_t n)
{
    char * ret = d;
    char * p = d;
    const char * q = s;
    while (n--)
        *p++ = *q++;
    return ret;
}

void *memset(void *s, int c, size_t n)
{
   unsigned char *dst = s;
    while (n > 0) {
        *dst = (unsigned char) c;
        dst++;
        n--;
    }
    return s;
}