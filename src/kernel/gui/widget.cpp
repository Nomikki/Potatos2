#include <gui/widget.hpp>

using namespace os::gui::widget;
using namespace os::driver::Keyboard;

Widget::Widget(Widget *parent,
               int32_t x,
               int32_t y,
               int32_t w,
               int32_t h,
               uint8_t r,
               uint8_t g,
               uint8_t b)
    : KeyboardEventHandler()
{
  this->parent = parent;
  this->x = x;
  this->y = y;
  this->w = w;
  this->h = h;

  this->r = r;
  this->g = g;
  this->b = b;
  this->focussable = true;
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

void Widget::Draw(os::driver::Vesa *gc)
{
  int absX = 0;
  int absY = 0;
  ModelToScreen(absX, absY);
  gc->FillRect(absX, absY, w, h, r, g, b);
}

void Widget::OnMouseDown(int32_t x, int32_t y, uint8_t button)
{
  GetFocus(this);
}

void Widget::OnMouseUp(int32_t x, int32_t y, uint8_t button)
{
}

void Widget::OnMouseMove(int32_t oldx, int32_t oldy, int32_t x, int32_t y)
{
}

// compositewidget
CompositeWidget::CompositeWidget(Widget *parent,
                                 int32_t x,
                                 int32_t y,
                                 int32_t w,
                                 int32_t h,
                                 uint8_t r,
                                 uint8_t g,
                                 uint8_t b)
    : Widget(parent, x, y, w, h, r, g, b)
{
  focussedChild = 0;
  numChildren = 0;
}

CompositeWidget::~CompositeWidget()
{
}

void CompositeWidget::GetFocus(Widget *widget)
{
  this->focussedChild = widget;
  if (parent != 0)
  {
    parent->GetFocus(this);
  }
}

bool CompositeWidget::AddChild(Widget *child)
{
  if (numChildren >= 100)
    return false;

  children[numChildren++] = child;
  return true;
}

void CompositeWidget::Draw(os::driver::Vesa *gc)
{
  Widget::Draw(gc);

  for (int i = numChildren - 1; i >= 0; --i)
    children[i]->Draw(gc);
}

void CompositeWidget::OnMouseDown(int32_t x, int32_t y, uint8_t button)
{

  for (int i = 0; i < numChildren; ++i)
    if (children[i]->ContainsCoordinate(x - this->x, y - this->y))
    {
      children[i]->OnMouseDown(x - this->x, y - this->y, button);
      break;
    }
}

void CompositeWidget::OnMouseUp(int32_t x, int32_t y, uint8_t button)
{
  for (int i = 0; i < numChildren; i++)
  {
    if (children[i]->ContainsCoordinate(x - this->x, y - this->y))
    {
      children[i]->OnMouseUp(x - this->x, y - this->y, button);
      break;
    }
  }
}

void CompositeWidget::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newX, int32_t newY)
{
  int firstchild = -1;
  for (int i = 0; i < numChildren; ++i)
    if (children[i]->ContainsCoordinate(oldx - this->x, oldy - this->y))
    {
      children[i]->OnMouseMove(oldx - this->x, oldy - this->y, newX - this->x, newY - this->y);
      firstchild = i;
      break;
    }

  for (int i = 0; i < numChildren; ++i)
    if (children[i]->ContainsCoordinate(newX - this->x, newY - this->y))
    {
      if (firstchild != i)
        children[i]->OnMouseMove(oldx - this->x, oldy - this->y, newX - this->x, newY - this->y);
      break;
    }
}

void CompositeWidget::OnKeyDown(char key)
{
  if (focussedChild != 0)
    focussedChild->OnKeyDown(key);
}

void CompositeWidget::OnKeyUp(char key)
{
  if (focussedChild != 0)
    focussedChild->OnKeyDown(key);
}

bool Widget::ContainsCoordinate(int32_t x, int32_t y)
{
  return (x >= this->x && y >= this->y && x <= this->x + this->w && y <= this->y + this->h);
}
