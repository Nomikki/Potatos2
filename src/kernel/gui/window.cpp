#include <gui/window.hpp>

using namespace os::gui::widget;
using namespace os::gui::window;

Window::Window(Widget *parent,
               int32_t x,
               int32_t y,
               int32_t w,
               int32_t h,
               uint8_t r,
               uint8_t g,
               uint8_t b)
    : CompositeWidget(parent, x, y, w, h, r, g, b)
{
  dragging = false;
}

Window::~Window()
{
}

void Window::Draw(os::driver::Vesa *gc)
{
  CompositeWidget::Draw(gc);
  gc->DrawRect(x-1, y-1, w+1, h+1, 0, 0, 0);
}

void Window::OnMouseDown(int32_t x, int32_t y, uint8_t button)
{
  dragging = button == 1;
  CompositeWidget::OnMouseDown(x, y, button);
}

void Window::OnMouseUp(int32_t x, int32_t y, uint8_t button)
{
  dragging = false;
  CompositeWidget::OnMouseUp(x, y, button);
}

void Window::OnMouseMove(int32_t oldX, int32_t oldY, int32_t newX, int32_t newY)
{
  if (dragging) 
  {
    this->x += newX - oldX;
    this->y += newY - oldY;
  }

  CompositeWidget::OnMouseMove(oldX, oldY, newX, newY);
}