
#include "common/stdint.h"
#include "common/stdio.h"
#include "common/hardware/vga.h"

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

  /*
  for (int i = 0; i < 48; i++)
  {
    printf("i: %i -> %X\n", i, i);
  }

  for (int i = 0; i < 80 * 12 + 4; i++)
  {
    printf("%c", 'a' + (i % 26));
  }
  */

  //__asm__("hlt");
  while (1)
    ;
}
