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

#endif // __MEMORYMANAGEMENT_H__