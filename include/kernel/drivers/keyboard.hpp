#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <stdint.h>
#include <communication/ports.hpp>
#include <communication/idt.hpp>
#include <drivers/driver.hpp>

namespace os::driver::Keyboard
{

  class KeyboardEventHandler
  {
  public:
    KeyboardEventHandler();
    ~KeyboardEventHandler();

    virtual void OnKeyDown(char key);
    virtual void OnKeyUp(char key);
  };

  class KeyboardDriver : public os::communication::InterruptHandler, public os::driver::Driver
  {
    os::communication::Port8Bit dataPort;
    os::communication::Port8Bit commandPort;
    bool keycodes[128];
    KeyboardEventHandler *eventHandler;

    void HandleKey(uint8_t key, bool pressed);

  public:
    KeyboardDriver(os::communication::InterruptManager *manager, KeyboardEventHandler *eventHandler);
    ~KeyboardDriver();
    virtual uint32_t HandleInterrupt(uint32_t esp);
    virtual void Activate();
  };
};

#endif // __KEYBOARD_H__