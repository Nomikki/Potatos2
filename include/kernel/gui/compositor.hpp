#ifndef __COMPOSITOR_H__
#define __COMPOSITOR_H__

#include <stdint.h>

namespace os::gui::compositor
{
  

  class Compositor
  {
    uint16_t x, y, w, h;

  public:
    Compositor();
    ~Compositor();
  };

};

#endif // __COMPOSITOR_H__