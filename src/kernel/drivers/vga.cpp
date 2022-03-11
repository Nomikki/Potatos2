#include <drivers/vga.h>

static uint16_t *videoAddr = (uint16_t *)0xB8000;
static uint16_t videoAddrIndex = 0;

struct vgaCursor
{
  int x;
  int y;
};

vgaCursor vCursor;


static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}
 

void vga_init()
{
  vCursor.x = 0;
  vCursor.y = 0;

  
  uint16_t color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

  for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
  {
    videoAddr[i] = ' ' | color << 8;
  }
}

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
  for (int y = 0; y < VGA_HEIGHT-1; y++)
  {
    for (int x = 0; x < VGA_WIDTH; x++)
    {
      videoAddr[VGA_WIDTH * y + x] = videoAddr[VGA_WIDTH * (y + 1) + x];
      videoAddr[VGA_WIDTH * (y + 1) + x] = 0;
    }
  }
}

void vga_putEntry(int character, uint16_t color)
{

  if (vCursor.y > VGA_HEIGHT-1)
  {
    vCursor.y = VGA_HEIGHT-1;
    vCursor.x = 0;

    vga_scrollLine();
  }

  if (character == '\n')
  {
    vCursor.y++;
    vCursor.x = 0;
    return;
  }

  videoAddrIndex = VGA_WIDTH * vCursor.y + vCursor.x;
  videoAddr[videoAddrIndex] = character | color;
  vCursor.x++;

  if (vCursor.x >= VGA_WIDTH)
  {
    vCursor.x = 0;
    vCursor.y++;
  }
}