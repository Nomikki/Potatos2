
#include "common/stdint.h"
#include "common/stdio.h"

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
  //printf("ruaah %s", "s");
  puts("Papin peruna");

  while (1)
    ;
}
