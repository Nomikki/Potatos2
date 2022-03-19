.set IRQ_BASE, 0x20

.section .text
.extern _ZN2os13communication16InterruptManager15HandleInterruptEjhj



.global _ZN2os13communication16InterruptManager22IgnoreInterruptRequestEv

.macro HandleException num
.global _ZN2os13communication16InterruptManager19HandleException\num\()Ev
_ZN2os13communication16InterruptManager19HandleException\num\()Ev:
  movb $\num, (interruptnumber)
  pushl $0
  jmp int_bottom
.endm

# FIXME: h
.macro HandleExceptionErr num
.global _ZN2os13communication16InterruptManager19HandleException\num\()Ev
_ZN2os13communication16InterruptManager19HandleException\num\()Ev:
  movb $\num, (interruptnumber)
  pushl $0
  jmp int_bottom
.endm

.macro HandleInterruptRequest num
.global _ZN2os13communication16InterruptManager26HandleInterruptRequest\num\()Ev
_ZN2os13communication16InterruptManager26HandleInterruptRequest\num\()Ev:
  movb $\num + IRQ_BASE, (interruptnumber)
  pushl $0
  jmp int_bottom
.endm



HandleInterruptRequest 0x00 # timer
HandleInterruptRequest 0x01 # keyboard
HandleInterruptRequest 0x0C # mouse

HandleException 0x00 # div by zero
HandleException 0x01 # debug
HandleException 0x02 # nmi
HandleException 0x03 # breakpoint
HandleException 0x04 # overflow
HandleException 0x05 # bound range exceeded
HandleException 0x06 # invalid opcode
HandleException 0x07 # dev not available
HandleExceptionErr 0x08 # double fault
HandleException 0x09 # ?
HandleExceptionErr 0x0A # invalid tss
HandleExceptionErr 0x0B # segment not present
HandleExceptionErr 0x0C # stack segment fault
HandleExceptionErr 0x0D # general protection fault
HandleExceptionErr 0x0E # page fault
HandleException 0x0F # reserved
HandleException 0x10 # x87 floating point exception
HandleExceptionErr 0x11 # align check
HandleException 0x12
HandleException 0x13

int_bottom:
  # save registers
    pushl %ebp
    pushl %edi
    pushl %esi

    pushl %edx
    pushl %ecx
    pushl %ebx
    pushl %eax
    # call C++ Handler
    
    pushl %esp
    push (interruptnumber)
    pushl $0;
  call _ZN2os13communication16InterruptManager15HandleInterruptEjhj
  #add %esp, 6
    mov %eax, %esp # switch the stack

    # restore registers
    popl %eax
    popl %ebx
    popl %ecx
    popl %edx

    popl %esi
    popl %edi
    popl %ebp  
    add $4, %esp


_ZN2os13communication16InterruptManager22IgnoreInterruptRequestEv:

  iret

.data
  interruptnumber: .byte 0
