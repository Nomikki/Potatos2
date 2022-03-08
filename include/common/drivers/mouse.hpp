#ifndef _MOUSE_HPP
#define _MOUSE_HPP

#include "common/stdint.h"
#include "common/communication/ports.hpp"
#include "common/communication/idt.hpp"
#include "common/drivers/driver.hpp"

class MouseDriver : public InterruptHandler, public Driver
{
#define MOUSE_IRQ 0x2c
#define DATA_PORT 0x60
#define COMMAND_PORT 0x64
#define ENABLE_AUXILIARY_DEVICE 0xA8
#define GET_COMPAQ_STATUS_BYTE 0x20
#define SET_COMPAQ_STATUS_BYTE 0x60
#define WAIT_COMMAND_PORT 0xD4

#define RESET 0xFF
#define RESEND 0xFE
#define SET_DEFAULTS 0xF6
#define SET_SAMPLERATE 0xF3
#define ENABLE_PACKED_STREAMING 0xF4
#define DISABLE_PACKED_STREAMING 0xF5

  Port8Bit dataPort;
  Port8Bit commandPort;

  uint8_t buffer[3];
  uint8_t offset;
  uint8_t buttons;

  void WaitACK();

public:
  MouseDriver(InterruptManager *manager);
  ~MouseDriver();
  virtual uint32_t HandleInterrupt(uint32_t esp);
  
  virtual void Activate();
};

#endif