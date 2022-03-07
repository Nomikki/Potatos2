

typedef unsigned int uint32_t;

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
  
  while (1)
    ;
}
