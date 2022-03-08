
#include "common/stdint.h"
#include "common/stdio.h"
#include "common/hardware/vga.h"
#include "memory/gdt.hpp"
#include "common/communication/idt.hpp"
#include "common/hardware/keyboard.hpp"


typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;

extern "C" void call_constructors()
{
  for (constructor *i = &start_ctors; i != &end_ctors; i++)
    (*i)();
}

extern "C" void kernel_main(const uint32_t sizeOfMemory, uint32_t multibootMagic, uint32_t stackSize, uint32_t stackStart)
{
  vga_init();
  GlobalDescriptorTable gdt;
  gdt.init();
  InterruptManager idt(&gdt);
  KeyboardDriver keyboard(&idt);

  idt.Activate();

  
  printf("Size of memory: %i Mb\n", sizeOfMemory / 1024 / 1024);
  printf("Size of stack: %i Kb\n", stackSize / 1024);
  printf("Start of stack: 0x%X\n", stackStart);
  

  while (1)
    ;
}
