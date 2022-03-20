/*
  Kun tietokoneen käynnistää, pic ei päästä ulkopuolisesta raudasta mitään signaalia
  cpu:lle läpi. Eli hiiri, näppäimistö jne. Picille pitää erikseen kertoa että voi päästää läpi.
  Mutta ensin pitää asettaa IDT kuntoon ja vasta sen jälkeen annetaan lupa
  käyttää sitä.

  IDT sisältää 256 entryä.
  keskeytysluku on 8 bittinen kokonaisluku
  handlker: funktio-osoite
  liput
  segmentti
  pääsyoikeudet (0 - 3)

  //på finska
  IDT-himmeli:
  - Konstruktorissa asennetaan tarvittavat keskeytykset ja mihin funktioon ne
    johtaa (esim HandleInterruptRequest0x00)
  - Kun keskeytys laukaistaan, hypätään idt_stub.s:n puolella oleviin
    HandleInterrupt-makroihin joissa asetetaan pinoon nykyiset rekisteriarvot
    ja tehdään loikka InterruptManager::HandleInterrupt-metodiin
  - kyseisessä metodissa hypätään lopulta DoHandleInterruptiin (pieni ruma
    kikka, koska meillä on staattisia funktioita) jolla päästään viimein
    käsittelemään keskeytykset

  - Jos meillä on handleri kyseisen keskeytysnumeron kohdalla, hypätään siihen
    (näin voidaan bindaa eri laitteille keskeytykset)
    - Muussa tapauksessa jos keskeytys on kaikkea muuta kuin 32 (pit timer),
      huudellaan hanskaamattomasta keskeytyksestä

  - Jos keskeytysnumero on alta väliltä 32...48 (0x20...0x30), kyse on
    remapatusta IRQ:sta. Kerrotaan siihen suuntaan että ollaan valmiita
    jatkamaan.
  - Lopuksi aina palautetaan ESP, jotta tiedetään missä meidän pino menee
    (jotta voidaan palauttaa oikeat rekisteriarvot)

  - Ajurinäkökulmasta jos halutaan käyttää keskeytyksiä:
    Ajurissa on oltava  InterruptHandler (perittynä) ja InterruptManager.
    Handlerille kerromme mikä keskeytysluku ja mikä InterruptManager.
  - Handleri lisää itsensä sisältä käsin InterruptManageriin. Ja koska handler
    on peritty, voidaan sen virtuaalista HandleInterrupt-funktiota kutsua,
    jolloinka ajurissa oleva vastaava kutsu kutsutaan.


*/
#include <communication/idt.hpp>
#include <stdio.h>

using namespace os::communication;

InterruptHandler::InterruptHandler(uint8_t interruptNumber, InterruptManager *interruptManager)
{
  this->interruptNumber = interruptNumber;
  this->interruptManager = interruptManager;
  // and put it itself to interrupt manager
  interruptManager->handlers[interruptNumber] = this;
}

InterruptHandler::~InterruptHandler()
{
  if (interruptManager->handlers[interruptNumber] == this)
    interruptManager->handlers[interruptNumber] = 0;
}

uint32_t InterruptHandler::HandleInterrupt(uint32_t esp)
{
  return esp;
}

//--------------------------
InterruptManager::GateDescriptor InterruptManager::interruptDecriptorTable[256];

InterruptManager *InterruptManager::ActiveInterruptManager = 0;

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

InterruptManager::InterruptManager(os::memory::GlobalDescriptorTable *gdt, os::TaskManager *taskManager)
    : picMasterCommand(0x20),
      picMasterData(0x21),
      picSlaveCommand(0xA0),
      picSlaveData(0xA1)
{
  uint16_t CodeSegment = gdt->CodeSegmentSelector();
  this->taskManager = taskManager;

  const uint8_t IDT_INTERRUPT_GATE = 0x0E;

  for (uint16_t i = 0; i < 256; i++)
  {
    SetInterruptDescriptorTableEntry(i, CodeSegment, &IgnoreInterruptRequest, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
    handlers[i] = 0;
  }

  // set exceptions
  SetInterruptDescriptorTableEntry(0x00, CodeSegment, &HandleException0x00, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x01, CodeSegment, &HandleException0x01, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x02, CodeSegment, &HandleException0x02, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x03, CodeSegment, &HandleException0x03, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x04, CodeSegment, &HandleException0x04, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x05, CodeSegment, &HandleException0x05, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x06, CodeSegment, &HandleException0x06, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x07, CodeSegment, &HandleException0x07, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x08, CodeSegment, &HandleException0x08, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x09, CodeSegment, &HandleException0x09, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x0A, CodeSegment, &HandleException0x0A, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x0B, CodeSegment, &HandleException0x0B, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x0C, CodeSegment, &HandleException0x0C, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x0D, CodeSegment, &HandleException0x0D, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x0E, CodeSegment, &HandleException0x0E, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x0F, CodeSegment, &HandleException0x0F, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x10, CodeSegment, &HandleException0x10, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x11, CodeSegment, &HandleException0x11, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x12, CodeSegment, &HandleException0x12, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x13, CodeSegment, &HandleException0x13, 0 /*kernel space*/, IDT_INTERRUPT_GATE);

  /*
    If we get interrupt 0x20, we jump to handleInterruptRequest0x00
  */
  SetInterruptDescriptorTableEntry(0x20, CodeSegment, &HandleInterruptRequest0x00, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x21, CodeSegment, &HandleInterruptRequest0x01, 0 /*kernel space*/, IDT_INTERRUPT_GATE);
  SetInterruptDescriptorTableEntry(0x2C, CodeSegment, &HandleInterruptRequest0x0C, 0 /*kernel space*/, IDT_INTERRUPT_GATE);

  // before we load IDS, we must communicate with PICs
  // and tell to not ignore signals anymore (with command 0x11)
  // and remap irq (Add 32 or 0x20)
  // both pics can make 8 interrupts
  picMasterCommand.Write(0x11);
  picSlaveCommand.Write(0x11);

  picMasterData.Write(0x20); // 0x20...0x27
  picSlaveData.Write(0x28);  // 0x28...0x30

  // tell to master pic that you are the master
  // and tell to slave that you are the slave
  picMasterData.Write(0x04);
  picSlaveData.Write(0x02);

  picMasterData.Write(0x01);
  picSlaveData.Write(0x01);

  picMasterData.Write(0x00);
  picSlaveData.Write(0x00);

  // and finally, load idt
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
  if (ActiveInterruptManager != 0)
    ActiveInterruptManager->Deactivate();

  ActiveInterruptManager = this;
  __asm__ volatile("sti");
}

void InterruptManager::Deactivate()
{
  if (ActiveInterruptManager == this)
  {
    ActiveInterruptManager = 0;
    __asm__ volatile("cli");
  }

  ActiveInterruptManager = this;
  __asm__ volatile("sti");
}

uint32_t InterruptManager::HandleInterrupt(uint8_t interruptNumber, uint32_t esp)
{
  if (ActiveInterruptManager != 0)
    return ActiveInterruptManager->DoHandleInterrupt(interruptNumber, esp);
  return esp;
}

uint32_t InterruptManager::DoHandleInterrupt(uint8_t interruptNumber, uint32_t esp)
{

  if (handlers[interruptNumber] != 0)
  {
    esp = handlers[interruptNumber]->HandleInterrupt(esp);
  }
  else if (interruptNumber != 0x20)
  {
    // if its not timer interrupts, print value
    printf("PANIC!\n");

    if (interruptNumber == 0x00)
      printf("[#DE] [ ] DIV BY ZERO\n");
    else if (interruptNumber == 0x01)
      printf("[#DB] [ ] DEBUG\n");
    else if (interruptNumber == 0x02)
      printf("[ - ] [ ] NMI\n");
    else if (interruptNumber == 0x03)
      printf("[#BP] [ ] BREAKPOINT\n");
    else if (interruptNumber == 0x04)
      printf("[#OF] [ ] OVERFLOW\n");
    else if (interruptNumber == 0x05)
      printf("[#BR] [ ] BOUND RANGE EXCEEDED\n");
    else if (interruptNumber == 0x06)
      printf("[#UD] [ ] INVALID OPCODE\n");
    else if (interruptNumber == 0x07)
      printf("[#NM] [ ] DEV NOT AVAILABLE\n");
    else if (interruptNumber == 0x08)
      printf("[#DF] [E] DOUBLE FAULT\n");
    else if (interruptNumber == 0x09)
      printf("[ - ] [ ] ?\n");
    else if (interruptNumber == 0x0A)
      printf("[#TS] [E] INVALID TSS\n");
    else if (interruptNumber == 0x0B)
      printf("[#NP] [E] SEGMENT NOT PRESENT\n");
    else if (interruptNumber == 0x0C)
      printf("[#SS] [E] STACK SEGMENT FAULT\n");
    else if (interruptNumber == 0x0D)
      printf("[#GP] [E] GENERAL PROTECTION FAULT\n");
    else if (interruptNumber == 0x0E)
      printf("[#PF] [E] PAGE FAULT\n");
    else if (interruptNumber == 0x0F)
      printf("[ - ] [ ] RESERVED\n");
    else if (interruptNumber == 0X10)
      printf("[#MF] [ ] X87 FLOATING POINT EXCEPTION");
    else if (interruptNumber == 0x11)
      printf("[#AC] [E] ALIGN CHECK\n");
    else if (interruptNumber == 0x12)
      printf("[#MC] [ ] MACHINE CHECK\n");
    else if (interruptNumber == 0x13)
      printf("[#XM] [ ] SIMD FLOATING POINT ERROR\n");
    else
      printf("UNHANDLED INTERRUPT %i (0x%x)\n", interruptNumber, interruptNumber);

    CPUState *cpu = (CPUState *)esp;
    printf("EAX %X  EBX %X   ECX %X  EDX %X\n", cpu->eax, cpu->ebx, cpu->ecx, cpu->edx);
    printf("ESI %X  EDI %X   EBP %X\n", cpu->esi, cpu->edi, cpu->ebp);
    printf("EFLAGS %X [%B]\n", cpu->eflags, cpu->eflags);
    printf("EIP %X\n", cpu->eip);
    printf("ERROR %X\n", cpu->errorCode);

    printf("CS %x  ESP %X   SS %X\n", cpu->cs, cpu->esp, cpu->ss);

    while (1)
      ;
    ;
  }

  // timer interrupt
  if (interruptNumber == 0x20)
  {
    esp = (uint32_t)taskManager->Schedule((CPUState *)esp);
  }

  if (interruptNumber >= 0x20 && interruptNumber < 0x20 + 16)
  {
    // we only have to answer to hardware interrupts
    // we are good with that interrupt, so send command to pics, that we are ready to take more
    ActiveInterruptManager->picMasterCommand.Write(0x20);
    if (0x20 + 8 <= interruptNumber)
      ActiveInterruptManager->picSlaveCommand.Write(0x20);
  }

  return esp;
}