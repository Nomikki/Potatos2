#include <stdint.h>
#include <stdio.h>
#include <drivers/vga.hpp>
#include <memory/gdt.hpp>
#include <communication/idt.hpp>
#include <drivers/driver.hpp>
#include <drivers/keyboard.hpp>
#include <drivers/mouse.hpp>
#include <communication/pci.hpp>

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;

extern "C" void call_constructors()
{
  for (constructor *i = &start_ctors; i != &end_ctors; i++)
    (*i)();
}

// testing
class printfKeyboardEventHandler : public os::driver::Keyboard::KeyboardEventHandler
{
public:
  void OnKeyDown(char key)
  {
    printf("%c", key);
  }

  void OnKeyUp(char key)
  {
  }
};

// testing
class MouseToConsole : public os::driver::Mouse::MouseEventHandler
{
  int x, y;
  uint16_t *const VideoMemory = (uint16_t *)0xB8000;

public:
  MouseToConsole()
  {
    x = 0;
    y = 0;
  }

  void OnMouseMove(int offsetX, int offsetY)
  {
    VideoMemory[80 * y + x] = ((VideoMemory[80 * y + x] & 0xF000) >> 4) | ((VideoMemory[80 * y + x] & 0x0F00) << 4) | ((VideoMemory[80 * y + x] & 0x00FF));
    x += offsetX;
    y += offsetY;
    if (x < 0)
      x = 0;
    if (x > 79)
      x = 79;
    if (y < 0)
      y = 0;
    if (y > 24)
      y = 24;
    VideoMemory[80 * y + x] = ((VideoMemory[80 * y + x] & 0xF000) >> 4) | ((VideoMemory[80 * y + x] & 0x0F00) << 4) | ((VideoMemory[80 * y + x] & 0x00FF));
  }
};

extern "C" void kernel_main(const uint32_t sizeOfMemory, uint32_t multibootMagic, uint32_t stackSize, uint32_t stackStart)
{
  os::driver::VGA::vga_init();
  os::memory::GlobalDescriptorTable gdt;
  gdt.init();
  os::communication::InterruptManager idt(&gdt);

  printf("Size of memory: %i Mb\n", sizeOfMemory / 1024 / 1024);
  printf("Size of stack: %i Kb\n", stackSize / 1024);
  printf("Start of stack: 0x%X\n", stackStart);

  os::driver::DriverManager drvManager;
  printfKeyboardEventHandler kbHandler;
  MouseToConsole mouseHandler;
  os::driver::Keyboard::KeyboardDriver keyboard(&idt, &kbHandler);
  os::driver::Mouse::MouseDriver mouse(&idt, &mouseHandler);

  drvManager.AddDriver(&keyboard);
  drvManager.AddDriver(&mouse);

  drvManager.ActivateAll();

  idt.Activate();

  os::communication::PCI pci;
  pci.SelectDrivers(&drvManager);

  while (1)
    ;
}
