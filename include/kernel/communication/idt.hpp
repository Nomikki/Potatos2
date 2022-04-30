#ifndef __IDT_H__
#define __IDT_H__

#include <stdint.h>
#include <communication/ports.hpp>
#include <memory/gdt.hpp>
#include <multitasking.hpp>

namespace os::communication
{

  class InterruptManager;
  // base class
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
    os::TaskManager *taskManager;

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
    InterruptManager(os::memory::GlobalDescriptorTable *gdt, os::TaskManager *taskManager);
    ~InterruptManager();

    void Activate();
    void Deactivate();

    // which interrupt and current stack pointer
    static uint32_t HandleInterrupt(uint8_t interruptNumber, uint32_t esp);
    uint32_t DoHandleInterrupt(uint8_t interruptNumber, uint32_t esp);

    static void IgnoreInterruptRequest();
    static void HandleInterruptRequest0x00(); // timer
    static void HandleInterruptRequest0x01(); // keyboard
    static void HandleInterruptRequest0x02();
    static void HandleInterruptRequest0x03();
    static void HandleInterruptRequest0x04();
    static void HandleInterruptRequest0x05();
    static void HandleInterruptRequest0x06();
    static void HandleInterruptRequest0x07();
    static void HandleInterruptRequest0x08();
    static void HandleInterruptRequest0x09();
    static void HandleInterruptRequest0x0A();
    static void HandleInterruptRequest0x0B();
    static void HandleInterruptRequest0x0C(); // mouse
    static void HandleInterruptRequest0x0D();
    static void HandleInterruptRequest0x0E();
    static void HandleInterruptRequest0x0F();
    static void HandleInterruptRequest0x31();

    static void HandleException0x00();
    static void HandleException0x01();
    static void HandleException0x02();
    static void HandleException0x03();
    static void HandleException0x04();
    static void HandleException0x05();
    static void HandleException0x06();
    static void HandleException0x07();
    static void HandleException0x08();
    static void HandleException0x09();
    static void HandleException0x0A();
    static void HandleException0x0B();
    static void HandleException0x0C();
    static void HandleException0x0D();
    static void HandleException0x0E();
    static void HandleException0x0F();
    static void HandleException0x10();
    static void HandleException0x11();
    static void HandleException0x12();
    static void HandleException0x13();
  };

};

#endif // __IDT_H__