#ifndef __DESKTOP_H__
#define __DESKTOP_H__

#include <gui/widget.hpp>
#include <drivers/mouse.hpp>
#include <drivers/vesa.hpp>

namespace os::gui::desktop
{
  class Desktop : public os::gui::widget::CompositeWidget, public os::driver::Mouse::MouseEventHandler
  {
  protected:
    uint32_t MouseX;
    uint32_t MouseY;

    uint32_t width;
    uint32_t height;

  public:
    Desktop(uint32_t width, uint32_t height, uint8_t r, uint8_t g, uint8_t b);
    ~Desktop();

    void Draw(os::driver::Vesa *gc);
    void DrawCursor(os::driver::Vesa *gc, int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b);
    virtual void OnMouseDown(uint8_t button);
    virtual void OnMouseUp(uint8_t button);
    virtual void OnMouseMove(int x, int y);
  };
};
#endif // __DESKTOP_H__