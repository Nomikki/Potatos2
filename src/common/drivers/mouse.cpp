#include "common/drivers/mouse.hpp"
#include "common/stdio.h"

MouseDriver::MouseDriver(InterruptManager *manager)
    : InterruptHandler(MOUSE_IRQ, manager), //which interrupt number?
      dataPort(DATA_PORT),
      commandPort(COMMAND_PORT)
{
}

MouseDriver::~MouseDriver()
{
}

void MouseDriver::Activate()
{
  printf("Install mouse... ");
  offset = 0;
  buttons = 0;

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

uint32_t MouseDriver::HandleInterrupt(uint32_t esp)
{
    printf(".");

  uint8_t status = commandPort.Read();
  if (!(status & 0x20)) //if 6th bit isn't 1, return
    return esp;

  buffer[offset] = dataPort.Read();

  /*
  if (handler == 0)
    return esp;
    */

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

      //handler->OnMouseMove(mx * 0.1, my * 0.1);
    }

    for (uint8_t i = 0; i < 3; i++)
    {
      if ((buffer[0] & (0x1 << i)) != (buttons & (0x1 << i)))
      {
        if (buttons & (0x1 << i))
        {
          //handler->OnMouseUp(i + 1);
        }
        else
        {
          //handler->OnMouseDown(i + 1);
        }
      }
    }
    buttons = buffer[0];
  }

  return esp;
}
