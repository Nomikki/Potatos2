#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <gui/widget.hpp>
#include <drivers/mouse.hpp>
#include <drivers/vesa.hpp>

namespace os::gui::window
{
  class Window : public os::gui::widget::CompositeWidget
  {
  protected:
    bool dragging;

  public:
    Window(Widget *parent,
           int32_t x,
           int32_t y,
           int32_t w,
           int32_t h,
           uint8_t r,
           uint8_t g,
           uint8_t b);

    ~Window();

    virtual void Draw(os::driver::Vesa *gc);
    virtual void OnMouseDown(int32_t x, int32_t y, uint8_t button);
    virtual void OnMouseUp(int32_t x, int32_t y, uint8_t button);
    virtual void OnMouseMove(int32_t oldX, int32_t oldY, int32_t newX, int32_t newY);
  };
};
#endif // __WINDOW_H__