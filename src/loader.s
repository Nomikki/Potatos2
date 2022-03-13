
.set MAGIC,			0x1badb002
.set FLAGS,		 0x1 | 0x2 | 0x4
.set CHECKSUM, -(MAGIC + FLAGS)
.set HEADER_ADDR, 0
.set LOAD_ADDR, 0
.set LOAD_END_ADDR, 0
.set LOAD_BSS_END_ADDR, 0
.set ENTRY_ADDR, 0
.set MODE_TYPE, 0
.set WIDTH, 1024
.set HEIGHT, 768
.set DEPTH, 32

# Aloitetaan multiboot-osuudella. Eli alkuun muutamat olennaiset tieodt, jotta grub löytää tämän.
.section .multiboot
	.long MAGIC
	.long FLAGS
	.long CHECKSUM

	.long  HEADER_ADDR
	.long  LOAD_ADDR
	.long  LOAD_END_ADDR
	.long  LOAD_BSS_END_ADDR
	.long  ENTRY_ADDR

	.long MODE_TYPE
	.long WIDTH
	.long HEIGHT 
	.long DEPTH

#boot_page_table1:

.section .multiboot
loader:


.section .text
.extern kernel_main
.extern call_constructors
.global loader

	# Annetaan pinolle osoitteeksi kernel_stackin kohta.
	mov $kernel_stack, %esp 

 pushl $(_kernel_end)
	# Koska käytämme C++:aa, vaatii se tämän.
	# Eli kutsumme hieman C++-koodia ennen kuin hyppäämme main-funktioon.
	call call_constructors


	# xor %ebp, %ebp
	push %ebx

	call kernel_main
	
_stop:
	cli
	hlt
	jmp _stop

.section .bootstrap_stack, "aw", @nobits
kernel_stack_start:
# .space 1*1024*1024 #1 MiB
.space 16*1024 # 16 KiB
kernel_stack:
