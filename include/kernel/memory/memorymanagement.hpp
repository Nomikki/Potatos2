#ifndef __MEMORYMANAGEMENT_H__
#define __MEMORYMANAGEMENT_H__

#include <stdint.h>

namespace os::memory
{
  struct MemoryChunk
  {
    MemoryChunk *next;
    MemoryChunk *prev;
    bool allocated;
    size_t size;
  };

  class MemoryManager
  {
  protected:
    MemoryChunk *first;

  public:
    static MemoryManager *activeMemoryManager;

    MemoryManager(size_t startAddress, size_t size);
    ~MemoryManager();

    void *malloc(size_t size);
    void free(void *ptr);
  };

};

// c++ style new/delete operators ♥
void *operator new(size_t size);
void *operator new[](size_t size);

// placement new
void *operator new(size_t size, void *ptr);
void *operator new[](size_t size, void *ptr);

void operator delete(void *ptr);
void operator delete[](void *ptr);

#endif // __MEMORYMANAGEMENT_H__