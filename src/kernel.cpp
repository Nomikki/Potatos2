#include <multiboot.h>
#include <stdint.h>
#include <stdio.h>
#include <drivers/vga.hpp>
#include <memory/gdt.hpp>
#include <communication/idt.hpp>
#include <multitasking.hpp>
#include <drivers/driver.hpp>
#include <drivers/keyboard.hpp>
#include <drivers/mouse.hpp>
#include <communication/pci.hpp>
#include <drivers/vesa.hpp>
#include <gui/desktop.hpp>
#include <gui/window.hpp>

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;

extern "C" void call_constructors()
{
  for (constructor *i = &start_ctors; i != &end_ctors; i++)
    (*i)();
}

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

  os::TaskManager taskManager;
  // os::Task task1(&gdt, taskA);
  // os::Task task2(&gdt, taskB);
  // taskManager.AddTask(&task1);
  // taskManager.AddTask(&task2);

  os::communication::InterruptManager idt(&gdt, &taskManager);

  os::communication::PCI pci;
  printf("upper memory: %i MB\n", mb_info->mem_upper / 1024);
  printf("kernelEnd: 0x%X\n", kernelEnd);

  os::driver::DriverManager drvManager;
 

  // text mode
  if (flags == 3)
  {
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

     drvManager.ActivateAll();
    pci.SelectDrivers(&drvManager, &idt);

    idt.Activate();

    while (1)
    {
      desktop.Draw(&vesa);
      vesa.Swap();
    }
  }
}
