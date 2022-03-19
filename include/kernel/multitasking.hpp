#ifndef __MULTITASKING_H__
#define __MULTITASKING_H__

#include <stdint.h>
#include <memory/gdt.hpp>

namespace os
{

  struct CPUState
  {

    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;

    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    /*
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    */
    uint32_t errorCode;

    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;

  } __attribute((packed));

  class Task
  {
    friend class TaskManager;

  private:
    uint8_t stack[4096];
    CPUState *cpuState;

  public:
    Task(os::memory::GlobalDescriptorTable *gdt, void entryPoint());
    ~Task();
  };

  class TaskManager
  {
  private:
    Task *tasks[256];
    int numTasks;
    int currentTask;

  public:
    TaskManager();
    ~TaskManager();
    bool AddTask(Task *task);
    CPUState *Schedule(CPUState *cpuState);
    ;
  };

};

#endif // __MULTITASKING_H__