#ifndef __VESA_H__
#define __VESA_H__

#include <stdint.h>

namespace os::driver
{

  class Vesa
  {
    uint32_t width;
    uint32_t height;
    uint8_t depth;
    unsigned int *buffer;
    unsigned int *doublebuffer;

  public:
    Vesa(uint32_t w, uint32_t h, uint32_t dbufferAddress);
    ~Vesa();
    void PutPixel(int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b);
    void Clear(uint8_t r, uint8_t g, uint8_t b);
    void FillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t r, uint8_t g, uint8_t b);
    void DrawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t r, uint8_t g, uint8_t b);
    void Swap();
  };


};

#endif // __VESA_H__