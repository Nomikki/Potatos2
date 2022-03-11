#include <stdio.h>
#include <drivers/vga.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>

int putchar(int character)
{
  uint16_t color = (uint16_t)VGA_COLOR_LIGHT_GREY << 8;
  vga_putEntry(character, color);
  return 0;
}

int puts(const char *str)
{
  for (int i = 0; i < strlen(str); i++)
  {
    putchar(str[i]);
  }
  return 0;
}

static bool print(const char *data, size_t length)
{
  const uint8_t *bytes = (const uint8_t *)data;
  for (size_t i = 0; i < length; i++)
  {
    if (putchar(bytes[i]) == EOF)
    {
      return false;
    }
  }
  return true;
}

int int2char(int32_t iNumber)
{
  if (iNumber == 0)
  {
    print("0", 1);
    return 1;
  }

  int iNumbersCount = 0;
  int iTmpNum = iNumber;
  while (iTmpNum)
  {
    iTmpNum /= 10;
    iNumbersCount++;
  }

  char buffer[32] = {0};

  for (int i = iNumbersCount - 1; i >= 0; i--)
  {
    buffer[i] = (char)((iNumber % 10) | 48);
    iNumber /= 10;
  }

  //buffer[iNumbersCount-1] = 0;
  print(buffer, iNumbersCount);
  return iNumbersCount;
}

int uint2char(uint32_t iNumber)
{
  if (iNumber == 0)
  {
    print("0", 1);
    return 1;
  }

  int iNumbersCount = 0;
  int iTmpNum = iNumber;
  while (iTmpNum)
  {
    iTmpNum /= 10;
    iNumbersCount++;
  }

  char buffer[32] = {0};

  for (int i = iNumbersCount - 1; i >= 0; i--)
  {
    buffer[i] = (char)((iNumber % 10) | 48);
    iNumber /= 10;
  }

  printf(buffer, iNumbersCount);
  return iNumbersCount;
}

int printBinary(int num)
{
  char binText[32] = {0};
  char binText2[32] = {0};
  int a = 0;
  while (true)
  {
    int b = num % 2;
    if (b == 0)
      binText[a++] = '0';
    if (b == 1)
      binText[a++] = '1';

    num /= 2;
    if (num < 1)
      break;
  }
  for (int i = 0; i < a; i++)
  {
    binText2[i] = binText[a - i - 1];
  }
  print(binText2, a);

  return a;
}

int printBigBinary(int num)
{
  //1337 -> 10100111001
  char binText[32] = {0};
  for (int i = 0; i < 32; i++)
  {
    binText[i] = '0';
  }

  int a = 32;
  while (true)
  {
    int b = num % 2;
    if (b == 0)
      binText[--a] = '0';
    if (b == 1)
      binText[--a] = '1';

    num /= 2;
    if (num <= 0)
      break;
  }
  print(binText, 32);
  return 32;
}

void printfHex(uint8_t num)
{
  char *foo = "00";
  char *hex = "0123456789ABCDEF";
  foo[0] = hex[(num >> 4) & 0xF];
  foo[1] = hex[num & 0xF];
  print(foo, 2);
}

void printfHex32(uint32_t key)
{
  printfHex((key >> 24) & 0xFF);
  printfHex((key >> 16) & 0xFF);
  printfHex((key >> 8) & 0xFF);
  printfHex(key & 0xFF);
}

int32_t printf(const char *__restrict format, ...)
{
  va_list parameters;
  va_start(parameters, format);

  int written = 0;

  while (*format != '\0')
  {
    size_t maxrem = INT_MAX - written;

    if (format[0] != '%' || format[1] == '%')
    {
      if (format[0] == '%')
        format++;

      size_t amount = 1;
      if (maxrem < amount)
      {
        //todo, set errno to overflow.
        return -1;
      }
      if (!print(format, amount))
        return -1;
      format += amount;
      written += amount;
      continue;
    }

    const char *format_begun_at = format++;

    //put single character
    if (*format == 'c')
    {
      format++;
      char c = (char)va_arg(parameters, int);
      if (!maxrem)
      {
        return -1;
      }
      if (!print(&c, sizeof(c)))
        return -1;
      written++;
    }
    //put string
    else if (*format == 's')
    {
      format++;
      const char *str = va_arg(parameters, const char *);
      size_t len = strlen(str);
      if (maxrem < len)
      {
        return -1;
      }
      if (!print(str, len))
        return -1;
      written += len;
    }
    //put integer
    else if (*format == 'i')
    {
      format++;
      int num = (int)va_arg(parameters, int);
      int len = int2char(num);
      written += len;
    }
    else if (*format == 'u')
    {
      format++;
      uint32_t num = (int)va_arg(parameters, uint32_t);
      int len = uint2char(num);
      written += len;
    }
    else if (*format == 'b')
    {
      format++;

      int num = (int)va_arg(parameters, int);
      if (maxrem < 8)
      {
        return -1;
      }
      int len = printBinary(num);

      written += len;
    }
    else if (*format == 'B')
    {
      format++;

      int num = (int)va_arg(parameters, int);
      if (maxrem < 8)
      {
        return -1;
      }
      int len = printBigBinary(num);

      written += len;
    }
    //put hexadecimals
    else if (*format == 'x')
    {
      format++;

      int num = (int)va_arg(parameters, int);
      if (maxrem < 2)
      {
        return -1;
      }
      printfHex(num);

      written += 2;
    }
    else if (*format == 'X')
    {
      format++;

      int num = (int)va_arg(parameters, int);
      if (maxrem < 8)
      {
        return -1;
      }
      printfHex32(num);

      written += 8;
    }
    else
    {
      format = format_begun_at;
      size_t len = strlen(format);
      if (maxrem < len)
      {
        return -1;
      }
      if (!print(format, len))
        return -1;
      written += len;
      format += len;
    }
  }

  va_end(parameters);
  return written;
}
