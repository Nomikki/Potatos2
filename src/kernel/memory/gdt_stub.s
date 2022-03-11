.section .text
.global _ZN2os6memory9gdt_flushEv

_ZN2os6memory9gdt_flushEv:
  jmp $0x08, $reload_CS

reload_CS:
  mov $0x10, %eax 
  mov %eax, %ds
  mov %eax, %es
  mov %eax, %fs
  mov %eax, %gs
  mov %eax, %ss
  ret 
  