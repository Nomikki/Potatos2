#ifndef _MOUSE_HPP
#define _MOUSE_HPP

#include <stdint.h>
#include <communication/ports.hpp>
#include <communication/idt.hpp>
#include <drivers/driver.hpp>

namespace os::driver::Mouse
{

#define MOUSE_IRQ 0x2c

  class MouseEventHandler
  {
  public:
    MouseEventHandler();
    ~MouseEventHandler();

    virtual void OnMouseDown(uint8_t button);
    virtual void OnMouseUp(uint8_t button);
    virtual void OnMouseMove(int x, int y);
    virtual void OnActivate();
  };

  class MouseDriver : public os::communication::InterruptHandler, public os::driver::Driver
  {
    os::communication::Port8Bit dataPort;
    os::communication::Port8Bit commandPort;

    uint8_t buffer[3];
    uint8_t offset;
    uint8_t buttons;

    void WaitACK();
    MouseEventHandler *eventHandler;

  public:
    MouseDriver(os::communication::InterruptManager *manager, MouseEventHandler *eventHandler);
    ~MouseDriver();
    virtual uint32_t HandleInterrupt(uint32_t esp);
    virtual void Activate();
  };

};

#endif