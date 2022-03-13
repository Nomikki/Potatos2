#include <gui/desktop.hpp>

using namespace os::gui::desktop;
using namespace os::gui::widget;
using namespace os::driver;

Desktop::Desktop(uint32_t width, uint32_t height, uint8_t r, uint8_t g, uint8_t b)
    : CompositeWidget(0, 0, 0, width, height, 255, 200, 20),
      MouseEventHandler()
{

  this->height = height;
  this->width = width;
  MouseX = width / 2;
  MouseY = height / 2;
}

Desktop::~Desktop()
{
}

void Desktop::DrawCursor(Vesa *gc, int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b)
{
  for (int j = 0; j < 8; j++)
    for (int i = 0; i < j; i++)
      gc->PutPixel(x + i, y + 8 - j, r, g, b);
}

void Desktop::Draw(Vesa *gc)
{
  CompositeWidget::Draw(gc);

  DrawCursor(gc, MouseX, MouseY, 0, 0, 0);
}

void Desktop::OnMouseDown(uint8_t button)
{
  CompositeWidget::OnMouseDown(MouseX, MouseY, button);
}

void Desktop::OnMouseUp(uint8_t button)
{
  CompositeWidget::OnMouseUp(MouseX, MouseY, button);
}

void Desktop::OnMouseMove(int x, int y)
{
  int32_t newMouseX = MouseX + x;
  int32_t newMouseY = MouseY + y;

  if (newMouseX < 0)
    newMouseX = 0;
  if (newMouseY < 0)
    newMouseY = 0;

  if (newMouseX >= width - 1)
    newMouseX = width - 1;
  if (newMouseY >= height - 1)
    newMouseY = height - 1;

  CompositeWidget::OnMouseMove(MouseX, MouseY, newMouseX, newMouseY);

  MouseX = newMouseX;
  MouseY = newMouseY;
}
