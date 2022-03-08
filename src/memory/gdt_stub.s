.section .text
.global _Z9gdt_flushv

_Z9gdt_flushv:
  jmp $0x08, $reload_CS

reload_CS:
  mov $0x10, %eax 
  mov %eax, %ds
  mov %eax, %es
  mov %eax, %fs
  mov %eax, %gs
  mov %eax, %ss
  ret 