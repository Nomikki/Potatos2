#ifndef _KEYBOARD_HPP
#define _KEYBOARD_HPP

#include <stdint.h>
#include <communication/ports.hpp>
#include <communication/idt.hpp>
#include <drivers/driver.hpp>

class KeyboardEventHandler
{
public:
  KeyboardEventHandler();
  ~KeyboardEventHandler();

  virtual void OnKeyDown(char key);
  virtual void OnKeyUp(char key);
};

class KeyboardDriver : public InterruptHandler, public Driver
{
  Port8Bit dataPort;
  Port8Bit commandPort;
  bool keycodes[128];
  KeyboardEventHandler *eventHandler;

  void HandleKey(uint8_t key, bool pressed);

public:
  KeyboardDriver(InterruptManager *manager, KeyboardEventHandler *eventHandler);
  ~KeyboardDriver();
  virtual uint32_t HandleInterrupt(uint32_t esp);
  virtual void Activate();
};

#endif