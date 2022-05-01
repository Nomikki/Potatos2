#include <multiboot.h>
#include <stdint.h>
#include <stdio.h>
#include <drivers/vga.hpp>
#include <memory/gdt.hpp>
#include <memory/memorymanagement.hpp>
#include <communication/idt.hpp>
#include <multitasking.hpp>
#include <drivers/driver.hpp>
#include <drivers/keyboard.hpp>
#include <drivers/mouse.hpp>
#include <communication/pci.hpp>
#include <drivers/vesa.hpp>
#include <gui/desktop.hpp>
#include <gui/window.hpp>
#include <drivers/AMD/am79c973.hpp>
#include <network/ethernetframe.hpp>

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;

extern uint32_t stackLeft();
extern uint32_t GetSizeOfStack();

void CheckStackStatus()
{
  uint32_t left = stackLeft() / 1024;
  uint32_t size = GetSizeOfStack() / 1024;
  float perc = (100.0 / size * left);

  printf("Stack left: %u kB / %u kB = %u% free\n", left, size, (int)perc );
}

extern "C" void call_constructors()
{
  for (constructor *i = &start_ctors; i != &end_ctors; i++)
    (*i)();
}

class PrintfKeyboardEventHandler : public os::driver::Keyboard::KeyboardEventHandler
{
public:
  void OnKeyDown(char c)
  {
    char *foo = " ";
    foo[0] = c;
    printf(foo);
  }
};

class MouseToConsole : public os::driver::Mouse::MouseEventHandler
{
  int8_t x, y;

public:
  MouseToConsole()
  {
    uint16_t *VideoMemory = (uint16_t *)0xb8000;
    x = 40;
    y = 12;
    VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4 | (VideoMemory[80 * y + x] & 0xF000) >> 4 | (VideoMemory[80 * y + x] & 0x00FF);
  }

  virtual void OnMouseMove(int xoffset, int yoffset)
  {
    static uint16_t *VideoMemory = (uint16_t *)0xb8000;
    VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4 | (VideoMemory[80 * y + x] & 0xF000) >> 4 | (VideoMemory[80 * y + x] & 0x00FF);

    x += xoffset;
    if (x >= 80)
      x = 79;
    if (x < 0)
      x = 0;
    y += yoffset;
    if (y >= 25)
      y = 24;
    if (y < 0)
      y = 0;

    VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4 | (VideoMemory[80 * y + x] & 0xF000) >> 4 | (VideoMemory[80 * y + x] & 0x00FF);
  }
};

void taskA()
{
  while (1)
  {
    printf("A");
    int32_t *ptr = 0x0C8000;
    for (int i = 0; i < 100000; i++)
    {
      *ptr = 0x0;
      ptr += i;
    }
  }
}

void taskB()
{
  while (1)
  {
    printf("B");
    uint32_t *ptr = 0x0C8000;
    for (int i = 0; i < 100000; i++)
    {
      *ptr = 0x0;
      ptr += i;
    }
  }
}

// extern "C" void kernel_main(uint32_t magic, uint32_t addr, uint32_t stackSize, uint32_t stackStart)
extern "C" void kernel_main(multiboot_info_t *mb_info, uint32_t kernelEnd, uint32_t flags)
{
  os::driver::VGA::vga_init();
  os::memory::GlobalDescriptorTable gdt;
  gdt.init();

  CheckStackStatus();

  size_t heapStartAddress = kernelEnd; // start heap after kernel
  size_t padding = 10 * 1024;
  size_t sizeOfHeap = mb_info->mem_upper * 1024 - heapStartAddress - padding;
  os::memory::MemoryManager memoryManager(heapStartAddress, sizeOfHeap);

  os::TaskManager taskManager;
  // os::Task task1(&gdt, taskA);
  // os::Task task2(&gdt, taskB);
  // taskManager.AddTask(&task1);
  // taskManager.AddTask(&task2);

  os::communication::InterruptManager idt(&gdt, &taskManager);

  os::communication::PCI pci;
  printf("Upper memory: %i MB\n", mb_info->mem_upper / 1024);
  printf("Addr of Kernel end: 0x%X\n", kernelEnd);
  printf("Addr of Heap: 0x%X\n", heapStartAddress);

  /*
    // allocation test
    for (int i = 0; i < 100; i++)
    {
      void *allocated = memoryManager.malloc(1024);
      printf("allocated: 0x%X\n", (size_t)allocated);

      void *allocated2 = memoryManager.malloc(1024);
      printf("allocated: 0x%X\n", (size_t)allocated2);

      memoryManager.free(allocated);

      void *allocated3 = memoryManager.malloc(10240);
      printf("allocated: 0x%X\n", (size_t)allocated3);

      memoryManager.free(allocated3);
      memoryManager.free(allocated2);
    }
   */
  os::driver::DriverManager drvManager;

  // text mode
  if (flags == 3)
  {
    PrintfKeyboardEventHandler kbhandler;
    os::driver::Keyboard::KeyboardDriver keyboard(&idt, &kbhandler);
    drvManager.AddDriver(&keyboard);

    MouseToConsole mousehandler;
    os::driver::Mouse::MouseDriver mouse(&idt, &mousehandler);
    drvManager.AddDriver(&mouse);

    pci.SelectDrivers(&drvManager, &idt);
    drvManager.ActivateAll();

    os::driver::am79c973 *eth0 = (os::driver::am79c973 *)(drvManager.mDrivers[2]);
    os::net::EthernetFrameProvider etherFrame(eth0);
    etherFrame.Send(0xFFFFFFFFFFFF, 0x0608, (uint8_t *)"FOO", 3);
    //  eth0->Send((uint8_t*)"Hello world", 12);

    idt.Activate();

    while (1)
      ;
  }
  else
  {

    os::gui::desktop::Desktop desktop(1024, 768, 255, 200, 20);
    os::driver::Keyboard::KeyboardDriver keyboard(&idt, &desktop);
    os::driver::Mouse::MouseDriver mouse(&idt, &desktop);

    drvManager.AddDriver(&keyboard);
    drvManager.AddDriver(&mouse);

    os::driver::Vesa vesa(1024, 768, kernelEnd);

    os::gui::window::Window window1(&desktop, 10, 10, 100, 100, 255, 255, 255);
    desktop.AddChild(&window1);

    os::gui::window::Window window2(&desktop, 64, 64, 200, 150, 0, 255, 0);
    desktop.AddChild(&window2);

    pci.SelectDrivers(&drvManager, &idt);
    drvManager.ActivateAll();

    idt.Activate();

    while (1)
    {
      desktop.Draw(&vesa);
      vesa.Swap();
    }
  }
}
