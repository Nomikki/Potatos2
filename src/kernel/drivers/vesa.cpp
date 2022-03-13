#include <drivers/vesa.hpp>

using namespace os::driver;

Vesa::Vesa(uint32_t w, uint32_t h)
{
  buffer = (unsigned int *)0xE0000000;
  //doublebuffer = (unsigned int *)(0x100000);

  width = w;
  height = h;
  depth = 32;

  
}

Vesa::~Vesa()
{
}

void Vesa::PutPixel(int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b)
{

  int i = width * y + x;
  if (x < 0 || y < 0 || x >= width || y >= height)
    return;
  buffer[i] = (r << 16) + (g << 8) + (b) + 0xff000000;
  
}

void Vesa::Clear(uint8_t r, uint8_t g, uint8_t b)
{
  
  uint32_t color = (r << 16) + (g << 8) + (b) + 0xff000000;
  for (uint32_t i = 0; i < width * height; i++)
    buffer[i] = color;
    
}

void Vesa::FillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t r, uint8_t g, uint8_t b)
{
  
  uint32_t color = (r << 16) + (g << 8) + (b) + 0xff000000;
  uint32_t offset = width * y + x;
  for (int j = y; j < y + h; j++)
  {
    for (int i = x; i < x + w; i++)
    {
      buffer[offset] = color;
      offset++;
    }
    offset += (width - w);
  }
  
}

void Vesa::Swap()
{
  /*
  for (uint32_t i = 0; i < width * height; i++)
    buffer[i] = doublebuffer[i];
*/
    
}