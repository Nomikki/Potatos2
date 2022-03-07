ENTRY(loader)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

SECTIONS
{
  . = 0x00100000;
  _kernel_start = .;
  
    .multiboot.data : {
    *(.multiboot.data)
  }

  .multiboot.text : {
    *(.multiboot.text)
  }

  . = 0xC0100000;

  .text ALIGN (4K) : AT (ADDR (.text) - 0xC0000000) 
  {
    *(.multiboot)
    *(.text*)
    *(.rodata)
  }

  .data ALIGN (4K) : AT (ADDR (.data) - 0xC0000000)
  {
    start_ctors = .;
    KEEP(*( .init_array ));
    KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
    end_ctors = .;

    *(.data)
  }

  .bss ALIGN (4K) : AT (ADDR (.bss) - 0xC0000000)
  {
    *(COMMON)
		*(.bss)
		*(.bootstrap_stack)
  }

  /DISCARD/ : { *(.fini_array*) *(.comment) }
  /*end = .; _end = .; __end = .;*/
  _kernel_end = .;

}