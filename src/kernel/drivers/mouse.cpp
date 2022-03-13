#include <drivers/mouse.hpp>
#include <stdio.h>

using namespace os::driver::Mouse;

MouseEventHandler::MouseEventHandler()
{
}

void MouseEventHandler::OnActivate()
{
}

void MouseEventHandler::OnMouseDown(uint8_t button)
{
}

void MouseEventHandler::OnMouseUp(uint8_t button)
{
}

void MouseEventHandler::OnMouseMove(int32_t x, int32_t y)
{
}

MouseDriver::MouseDriver(os::communication::InterruptManager *manager, MouseEventHandler *eventHandler)
    : InterruptHandler(MOUSE_IRQ, manager), // which interrupt number?
      dataPort(DATA_PORT),
      commandPort(COMMAND_PORT)
{
  this->eventHandler = eventHandler;
}

MouseDriver::~MouseDriver()
{
}

void MouseDriver::Activate()
{
  printf("Install mouse... ");
  offset = 0;
  buttons = 0;

  if (eventHandler != 0)
    eventHandler->OnActivate();

  commandPort.Write(ENABLE_AUXILIARY_DEVICE);
  WaitACK();

  commandPort.Write(GET_COMPAQ_STATUS_BYTE);

  uint8_t status = dataPort.Read() | 2;
  commandPort.Write(SET_COMPAQ_STATUS_BYTE);
  dataPort.Write(status);

  commandPort.Write(WAIT_COMMAND_PORT);
  dataPort.Write(ENABLE_PACKED_STREAMING);
  WaitACK();
  printf("ok\n");
}

void MouseDriver::WaitACK()
{
  while (1)
  {
    uint8_t dataRet = dataPort.Read();
    if (dataRet == 0xFA)
      break;
  }
}

/*
  Byte 1
   bit
    7: Y overflow #not useful
    6: X overflow #not useful
    5: Y sign bit #if set, do OR 0bFFFFFF00 for fast speed
    4: X Sign bit #if set, do OR 0bFFFFFF00 for fast speed
    3: Always 1
    2: Middle Button
    1: Right Button
    0: Left Button
  Byte 2
    X movement
  Byte 3
    Y movement

*/
uint32_t MouseDriver::HandleInterrupt(uint32_t esp)
{

  uint8_t status = commandPort.Read();
  if (!(status & 0x20)) // if 6th bit isn't 1, return
    return esp;

  buffer[offset] = dataPort.Read();

  if (eventHandler == 0)
    return esp;

  offset = (offset + 1) % 3;

  if (offset == 0)
  {

    bool xsign = false;
    bool ysign = false;

    if (buffer[0] & (0x1 << 5))
      ysign = true;
    if (buffer[0] & (0x1 << 4))
      xsign = true;

    if (buffer[1] != 0 || buffer[2] != 0)
    {
      int mx = (int32_t)buffer[1];
      int my = ((int32_t)buffer[2]);

      if (xsign)
      {
        mx |= 0xFFFFFF00;
      }
      if (ysign)
      {
        my |= 0xFFFFFF00;
      }

      if (mx < -255)
        mx = -255;
      if (mx > 255)
        mx = 255;
      if (my < -255)
        my = -255;
      if (my > 255)
        my = 255;

      my = -my;

      /*
      mx /= 2;
      my /= 2;
      */

      eventHandler->OnMouseMove(mx, my);
    }

    for (uint8_t i = 0; i < 3; i++)
    {
      if ((buffer[0] & (0x1 << i)) != (buttons & (0x1 << i)))
      {
        if (buttons & (0x1 << i))
        {
          eventHandler->OnMouseUp(i + 1);
        }
        else
        {
          eventHandler->OnMouseDown(i + 1);
        }
      }
    }
    buttons = buffer[0];
  }

  return esp;
}
