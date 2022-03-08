#ifndef _IDT_HPP
#define _IDT_HPP

#include "common/stdint.h"
#include "common/communication/ports.hpp"
#include "memory/gdt.hpp"

class InterruptManager;
//base class
class InterruptHandler
{
protected:
  uint8_t interruptNumber;
  InterruptManager *interruptManager;

  InterruptHandler(uint8_t interruptNumber, InterruptManager *interruptManager);
  ~InterruptHandler();

public:
  virtual uint32_t HandleInterrupt(uint32_t esp);
};
//-----------------------

class InterruptManager
{
  friend class InterruptHandler;
  InterruptHandler *handlers[256];

protected:
  static InterruptManager *ActiveInterruptManager;

  struct GateDescriptor
  {
    uint16_t handlerAddressLowBits;
    uint16_t gdt_codeSegmentSelector;
    uint8_t reserved;
    uint8_t access;
    uint16_t handlerAddressHighBits;
  } __attribute__((packed));

  static GateDescriptor interruptDecriptorTable[256];

  struct interruptDescriptorTablePointer
  {
    uint16_t size;
    uint32_t base;
  } __attribute__((packed));

  static void SetInterruptDescriptorTableEntry(
      uint8_t interruptNumber,
      uint16_t gdt_codeSegmentSelectorOffset,
      void (*handler)(),
      uint8_t DescriptorPrivilegeLevel,
      uint8_t DescriptorType);

  Port8BitSlow picMasterCommand;
  Port8BitSlow picMasterData;
  Port8BitSlow picSlaveCommand;
  Port8BitSlow picSlaveData;

public:
  InterruptManager(GlobalDescriptorTable *gdt);
  ~InterruptManager();

  void Activate();
  void Deactivate();

  //which interrupt and current stack pointer
  static uint32_t HandleInterrupt(uint8_t interruptNumber, uint32_t esp);
  uint32_t DoHandleInterrupt(uint8_t interruptNumber, uint32_t esp);

  static void IgnoreInterruptRequest();
  static void HandleInterruptRequest0x00(); //timer
  static void HandleInterruptRequest0x01(); //keyboard
  static void HandleInterruptRequest0x0C(); //mouse
  
};

#endif