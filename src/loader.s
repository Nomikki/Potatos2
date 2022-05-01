
.set MAGIC,			0x1badb002
.set ALIGN, 0x1
.set MEMINFO, 0x2
# .set VIDEO, 0x4


.ifdef VIDEO 
.set FLAGS,		 ALIGN | MEMINFO  | VIDEO 
.else
.set FLAGS,		 ALIGN | MEMINFO 
.endif 

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
.ifdef VIDEO 
	.long  HEADER_ADDR
	.long  LOAD_ADDR
	.long  LOAD_END_ADDR
	.long  LOAD_BSS_END_ADDR
	.long  ENTRY_ADDR

	.long MODE_TYPE
	.long WIDTH
	.long HEIGHT 
	.long DEPTH
.endif

# boot_page_table1:

.section .multiboot
loader:


.section .text
.extern kernel_main
.extern call_constructors
.global loader

# Higher half kernelin sisääntulo!
# Tämä osio on linkkerissä määritelty alkamaan kohdasta 0xC0000000.
4:
	# Nyt sivutus on täysin kunnossa ja käytössä.
	# Voimme unmapata identitymappauksen, koska emme tarvitse sitä enää.
	# Eli asetamme vain sisällöksi pelkkää nollaa
	# movl $0, boot_page_directory + 0

	# Ladataan nyt CR3 uudestaan (otetaan sisältö ECX-rekisteriin ja siirretään ECX CR3-rekisteriin)
	# Tämä aiheuttaa TLB-flushauksen, jolloinka MMU päivittyy.
	# movl %cr3, %ecx
	# movl %ecx, %cr3

	# Annetaan pinolle osoitteeksi kernel_stackin kohta.
	mov $kernel_stack, %esp 


	# Koska käytämme C++:aa, vaatii se tämän.
	# Eli kutsumme hieman C++-koodia ennen kuin hyppäämme main-funktioon.
	call call_constructors


	# addl $0xC03DF000, %ebx
	# movl $0x00010000, %ebx 

	# Annetaan parametrit kernelin main-funktiolle, alkaen oikealta vasemmalle.
	# push $kernel_stack_start # mistä alkaa pino
	# push %esp				 # pinon 'pohja'
	# push %ebx				 # pointer to multiboot structure
	# push %eax				 # magic number

	xor %ebp, %ebp
	pushl $FLAGS
	pushl $(_kernel_end)
	pushl %ebx
	xor %eax, %eax
	call kernel_main
	
_stop:
	cli
	hlt
	jmp _stop

.section .bootstrap_stack, "aw", @nobits
kernel_stack_start:
 .space 1*1024*1024 #1 MiB
# .space 16*1024 # 16 KiB
kernel_stack:
