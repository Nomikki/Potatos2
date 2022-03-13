#ifndef __WIDGET_H__
#define __WIDGET_H__

#include <stdint.h>
#include <drivers/vesa.hpp>

namespace os::gui::widget
{
  class Widget
  {
  protected:
    Widget *parent;
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;

    uint8_t r;
    uint8_t g;
    uint8_t b;

  public:
    Widget(Widget *parent,
           int32_t x,
           int32_t y,
           int32_t w,
           int32_t h,
           uint8_t r,
           uint8_t g,
           uint8_t b);
    ~Widget();

    virtual void GetFocus(Widget *widget);
    virtual void ModelToScreen(int32_t &x, int32_t y);

    virtual void Draw(os::drivers::Vesa *gc);
    virtual void OnMouseDown(int32_t x, int32_t y);
    virtual void OnMouseUp(int32_t x, int32_t y);
    virtual void OnMouseMove(int32_t oldx, int32_t oldy, int32_t x, int32_t y);

    virtual void OnKeyDown(int32_t x, int32_t y);
    virtual void OnKeyUp(int32_t x, int32_t y);
  };

};

#endif // __WIDGET_H__