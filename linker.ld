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

  /*. = 0xC0100000;*/

  .text ALIGN (4K) : AT (ADDR (.text) ) 
  {
    *(.multiboot)
    *(.text*)
    *(.rodata)
  }

  .data ALIGN (4K) : AT (ADDR (.data) )
  {
    start_ctors = .;
    KEEP(*( .init_array ));
    KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
    end_ctors = .;

    *(.data)
  }

  .bss ALIGN (4K) : AT (ADDR (.bss) )
  {
    *(COMMON)
		*(.bss)
		*(.bootstrap_stack)
  }

  /DISCARD/ : { *(.fini_array*) *(.comment) }

  _kernel_end = .;

}