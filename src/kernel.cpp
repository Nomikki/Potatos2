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

// extern "C" void kernel_main(uint32_t magic, uint32_t addr, uint32_t stackSize, uint32_t stackStart)
extern "C" void kernel_main(multiboot_info_t *mb_info)
{

  os::driver::VGA::vga_init();
  os::memory::GlobalDescriptorTable gdt;
  gdt.init();
  os::communication::InterruptManager idt(&gdt);

  os::gui::desktop::Desktop desktop(1024, 768, 255, 200, 20);

  os::driver::DriverManager drvManager;

  os::driver::Keyboard::KeyboardDriver keyboard(&idt, &desktop);
  os::driver::Mouse::MouseDriver mouse(&idt, &desktop);
  os::communication::PCI pci;

  drvManager.AddDriver(&keyboard);
  drvManager.AddDriver(&mouse);

  drvManager.ActivateAll();



  pci.SelectDrivers(&drvManager, &idt);

  os::driver::Vesa vesa(1024, 768);
  
  os::gui::window::Window window1(&desktop, 10, 10, 100, 100, 255, 255, 255);
  desktop.AddChild(&window1);

  os::gui::window::Window window2(&desktop, 64, 64, 200, 150, 0, 255, 0);
  desktop.AddChild(&window2);


  idt.Activate();
  while (1)
  {
    //vesa.Clear(255, 200, 20);
    desktop.Draw(&vesa);
  }
}
