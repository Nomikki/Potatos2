#include "common/hardware/vga.h"

static uint16_t *videoAddr = (uint16_t *)0xB8000;
static uint16_t videoAddrIndex = 0;

struct vgaCursor
{
  int x;
  int y;
};

vgaCursor vCursor;

void vga_getCursor(int &x, int &y)
{
  x = vCursor.x;
  y = vCursor.y;
}

void vga_setCursor(int x, int y)
{
  vCursor.x = x;
  vCursor.y = y;
}

void vga_scrollLine()
{
  for (int y = 0; y < 24; y++)
  {
    for (int x = 0; x < 80; x++)
    {
      videoAddr[80 * y + x] = videoAddr[80 * (y + 1) + x];
      videoAddr[80 * (y + 1) + x] = 0;
    }
  }
}

void vga_putEntry(int character, uint16_t color)
{

  if (vCursor.y > 24)
  {
    vCursor.y = 24;
    vCursor.x = 0;

    vga_scrollLine();
  }

  if (character == '\n')
  {
    vCursor.y++;
    vCursor.x = 0;
    return;
  }

  videoAddrIndex = 80 * vCursor.y + vCursor.x;
  videoAddr[videoAddrIndex] = character | color;
  vCursor.x++;

  if (vCursor.x >= 80)
  {
    vCursor.x = 0;
    vCursor.y++;
  }
}