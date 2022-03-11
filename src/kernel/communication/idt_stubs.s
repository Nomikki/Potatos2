.set IRQ_BASE, 0x20

.section .text
.extern _ZN2os13communication16InterruptManager15HandleInterruptEhj

.global _ZN2os13communication16InterruptManager22IgnoreInterruptRequestEv

.macro HandleException num
.global _ZN2os13communication16InterruptManager16HandleException\num\()Ev
_ZN2os13communication16InterruptManager16HandleException\num\()Ev:
  movb $\num + IRQ_BASE, (interruptnumber)
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

int_bottom:
  # save registers
    #pusha
    #pushl %ds
    #pushl %es
    #pushl %fs
    #pushl %gs
    
    pushl %ebp
    pushl %edi
    pushl %esi

    pushl %edx
    pushl %ecx
    pushl %ebx
    pushl %eax

    # load ring 0 segment register
    #cld
    #mov $0x10, %eax
    #mov %eax, %eds
    #mov %eax, %ees

    # call C++ Handler
    pushl %esp
    push (interruptnumber)
  call _ZN2os13communication16InterruptManager15HandleInterruptEhj
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
    #pop %gs
    #pop %fs
    #pop %es
    #pop %ds
    #popa
    
    add $4, %esp


_ZN2os13communication16InterruptManager22IgnoreInterruptRequestEv:

  iret

.data
  interruptnumber: .byte 0
