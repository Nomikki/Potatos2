#include <memory/memorymanagement.hpp>

using namespace os;
using namespace os::memory;

MemoryManager *MemoryManager::activeMemoryManager = 0;

MemoryManager::MemoryManager(size_t startAddress, size_t size)
{
  activeMemoryManager = this;
  if (size < sizeof(MemoryChunk))
  {
    first = 0;
  }
  else
  {
    first = (MemoryChunk *)startAddress;
  }
}

MemoryManager::~MemoryManager()
{
}

void *MemoryManager::malloc(size_t size)
{
}

void MemoryManager::free(void *ptr)
{
}
