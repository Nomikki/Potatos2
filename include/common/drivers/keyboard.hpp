#ifndef _KEYBOARD_HPP
#define _KEYBOARD_HPP

#include "common/stdint.h"
#include "common/communication/ports.hpp"
#include "common/communication/idt.hpp"
#include "common/drivers/driver.hpp"

class KeyboardDriver : public InterruptHandler, public Driver
{
  Port8Bit dataPort;
  Port8Bit commandPort;

  bool keys[128];
  void HandleKey(uint8_t key, bool pressedOrReleased);

public:
  

  KeyboardDriver(InterruptManager *manager);
  ~KeyboardDriver();
  virtual uint32_t HandleInterrupt(uint32_t esp);
  virtual void Activate();
};

#endif