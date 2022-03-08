/*
  Kun tietokoneen käynnistää, pic ei päästä ulkopuolisesta raudasta mitään signaalia cpu:lle läpi.
  Eli hiiri, näppäimistö jne. Picille pitää erikseen kertoa että voi päästää läpi.
  Mutta ensin pitää asettaa IDT kuntoon ja vasta sen jälkeen annetaan lupa käyttää sitä.

  IDT sisältää 256 entryä.
  keskeytysluku on 8 bittinen kokonaisluku
  handlker: funktio-osoite
  liput
  segmentti
  pääsyoikeudet (0 - 3)


*/
#include "common/communication/idt.hpp"
#include "common/stdio.h"

InterruptManager::GateDescriptor InterruptManager::interruptDecriptorTable[256];

void InterruptManager::SetInterruptDescriptorTableEntry(
    uint8_t interruptNumber,
    uint16_t gdt_codeSegmentSelectorOffset,
    void (*handler)(),
    uint8_t DescriptorPrivilegeLevel,
    uint8_t DescriptorType)
{
  const uint8_t IDT_DESC_PRESENT = 0x80;

  interruptDecriptorTable[interruptNumber].handlerAddressLowBits = ((uint32_t)handler) & 0xFFFF;
  interruptDecriptorTable[interruptNumber].handlerAddressHighBits = ((uint32_t)handler >> 16) & 0xFFFF;
  interruptDecriptorTable[interruptNumber].gdt_codeSegmentSelector = gdt_codeSegmentSelectorOffset;
  interruptDecriptorTable[interruptNumber].access = IDT_DESC_PRESENT | DescriptorType | (DescriptorPrivilegeLevel & 3 << 5);
  interruptDecriptorTable[interruptNumber].reserved = 0;
}

InterruptManager::InterruptManager(GlobalDescriptorTable *gdt)
    : picMasterCommand(0x20),
      picMasterData(0x21),
      picSlaveCommand(0xA0),
      picSlaveData(0xA1)
{
  uint16_t CodeSegment = gdt->CodeSegmentSelector();

  const uint8_t IDT_INTERRUPT_GATE = 0x0E;

  for (uint16_t i = 0; i < 256; i++)
    SetInterruptDescriptorTableEntry(i, CodeSegment, &IgnoreInterruptRequest, 0 /*kernel space*/, IDT_INTERRUPT_GATE);

  /*
    If we get interrupt 0x20, we jump to handleInterruptRequest0x00
  */
  SetInterruptDescriptorTableEntry(0x20, CodeSegment, &HandleInterruptRequest0x00, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x21, CodeSegment, &HandleInterruptRequest0x01, 0 /*kernel space*/, IDT_INTERRUPT_GATE);

  

  //before we load IDS, we must communicate with PICs
  //and tell to not ignore signals anymore (with command 0x11)
  //and remap irq (Add 32 or 0x20)
  //both pics can make 8 interrupts
  picMasterCommand.Write(0x11);
  picSlaveCommand.Write(0x11);

  picMasterData.Write(0x20); //0x20...0x27
  picSlaveData.Write(0x28);  //0x28...0x30

  //tell to master pic that you are the master
  //and tell to slave that you are the slave
  picMasterData.Write(0x04); 
  picSlaveData.Write(0x02);  

  picMasterData.Write(0x01); 
  picSlaveData.Write(0x01);  

  picMasterData.Write(0x00); 
  picSlaveData.Write(0x08);  


  //and finally, load idt 
  interruptDescriptorTablePointer idt;
  idt.size = 256 * sizeof(GateDescriptor) - 1;
  idt.base = (uint32_t)interruptDecriptorTable;
  __asm__ volatile("lidt %0"
                   :
                   : "m"(idt));
}

InterruptManager::~InterruptManager()
{
}

void InterruptManager::Activate()
{
  __asm__ volatile("sti");
}

uint32_t InterruptManager::handleInterrupt(uint8_t interruptNumber, uint32_t esp)
{
  printf("INTERRUPT %i (0x%x)\n", interruptNumber, interruptNumber);
  
  return esp;
}