#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "common/stdint.h"
#include "common/communication/ports.hpp"
#include "common/communication/idt.hpp"


class KeyboardDriver : public InterruptHandler
{
  Port8Bit dataPort;
  Port8Bit commandPort;

public:

  bool keys[128];

  KeyboardDriver(InterruptManager *manager);
  ~KeyboardDriver();
  virtual uint32_t HandleInterrupt(uint32_t esp);
  void HandleKey(uint8_t key, bool pressedOrReleased);

};

#endif