#include <multiboot.h>
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
    x = 1024 / 2;
    y = 768 / 2;
    DrawCursor(0, 0, 0);
  }

  void putPixel(int _x, int _y, uint8_t r, uint8_t g, uint8_t b)
  {
    static unsigned int *buffer = (unsigned int *)0xE0000000;
    int width = 1024;
    int height = 768;

    int i = width * _y + _x;
    if (_x < 0 || _y < 0 || _x >= width || _y >= height)
      return;
    buffer[i] = (r << 16) + (g << 8) + (b) + 0xff000000;
  }

  void DrawCursor(uint8_t r, uint8_t g, uint8_t b)
  {
    for (int j = 0; j < 8; j++)
      for (int i = 0; i < j; i++)
        putPixel(x + i, y + 8 - j, r, g, b);
  }

  void OnMouseMove(int offsetX, int offsetY)
  {

    int width = 1024;
    int height = 768;

    DrawCursor(255, 200, 25);

    x += offsetX;
    y += offsetY;
    if (x < 0)
      x = 0;
    if (x > width)
      x = width;
    if (y < 0)
      y = 0;
    if (y > height)
      y = height;

    DrawCursor(0, 0, 0);
  }
};

// extern "C" void kernel_main(uint32_t magic, uint32_t addr, uint32_t stackSize, uint32_t stackStart)
extern "C" void kernel_main(multiboot_info_t *mb_info)
{

  unsigned int *buffer = (unsigned int *)0xE0000000;
  uint8_t r = 255;
  uint8_t g = 200;
  uint8_t b = 25;

  for (int i = 0; i < 1024 * 768; i++)
  {
    buffer[i] = (r << 16) + (g << 8) + (b) + 0xff000000;
  }

  os::driver::VGA::vga_init();
  os::memory::GlobalDescriptorTable gdt;
  gdt.init();
  os::communication::InterruptManager idt(&gdt);

  // printf("magix = 0x%X\n", mb_info->magic);
  // printf("flags = 0x%X\n", mb_info->flags);
  // printf("checksum = 0x%X\n", mb_info->checksum);

  // if (mb_info->checksum != -(mb_info->magic + mb_info->flags))
  // printf("checksum failed.\n");

  // printf("mode: %u: %u * %u\n", mb_info->mode_type, mb_info->width, mb_info->height);

  printf("mem_lower = %uKB, mem_upper = %uMB\n", (unsigned)mb_info->mem_lower, (unsigned)mb_info->mem_upper / 1024);

  // printf("Size of memory: %i Mb\n", sizeOfMemory / 1024 / 1024);
  // printf("Size of stack: %i Kb\n", stackSize / 1024);
  // printf("Start of stack: 0x%X\n", stackStart);

  os::driver::DriverManager drvManager;
  printfKeyboardEventHandler kbHandler;
  MouseToConsole mouseHandler;
  os::driver::Keyboard::KeyboardDriver keyboard(&idt, &kbHandler);
  os::driver::Mouse::MouseDriver mouse(&idt, &mouseHandler);
  os::communication::PCI pci;

  drvManager.AddDriver(&keyboard);
  drvManager.AddDriver(&mouse);

  drvManager.ActivateAll();

  idt.Activate();

  pci.SelectDrivers(&drvManager, &idt);

  while (1)
    ;
}
