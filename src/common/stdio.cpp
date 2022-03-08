#include "common/stdio.h"

uint16_t *videoAddr = (uint16_t*)0xB8000;
uint16_t videoAddrIndex = 0;

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

int printf(const char *format, ...)
{
  return 0;
}

int putchar(int character)
{
  videoAddr[videoAddrIndex] = character | (uint16_t)VGA_COLOR_LIGHT_GREY << 8;
  videoAddrIndex++;
  if (videoAddrIndex > 80*25)
    videoAddrIndex = 0;
  return 0;
}

int puts(const char *str)
{
  int i = 0;
  while(1) {
    putchar(str[i]);
    i++;
    if (str[i] == 0)
      break;
  }

  return 0;
}