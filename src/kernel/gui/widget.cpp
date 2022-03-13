#include <gui/widget.hpp>

using namespace os::gui::widget;

Widget::Widget(Widget *parent,
               int32_t x,
               int32_t y,
               int32_t w,
               int32_t h,
               uint8_t r,
               uint8_t g,
               uint8_t b)
{
  this->parent = parent;
  this->x = x;
  this->y = y;
  this->w = w;
  this->h = h;

  this->r = r;
  this->g = g;
  this->b = b;
}

Widget::~Widget()
{
}

void Widget::GetFocus(Widget *widget)
{
  if (parent != 0)
    parent->GetFocus(widget);
}

void Widget::ModelToScreen(int32_t &x, int32_t y)
{
  if (parent != 0)
    parent->ModelToScreen(x, y);

  x += this->x;
  y += this->y;
}

void Widget::Draw(os::drivers::Vesa *gc)
{
  int absX = 0;
  int absY = 0;
  ModelToScreen(absX, absY);
  gc->FillRect(absX, absY, w, h, r, g, b);;
}

void Widget::OnMouseDown(int32_t x, int32_t y)
{
}

void Widget::OnMouseUp(int32_t x, int32_t y)
{
}

void Widget::OnMouseMove(int32_t oldx, int32_t oldy, int32_t x, int32_t y)
{
}

void Widget::OnKeyDown(int32_t x, int32_t y)
{
}

void Widget::OnKeyUp(int32_t x, int32_t y)
{
}