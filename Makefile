
# gfx = true
GPPPARAMS = -s -m32 -Iinclude/kernel -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings -fpermissive -fno-pic -ffreestanding -Wl,--build-id=none

ifeq ($(gfx), true)
ASPARAMS	= --32 --defsym VIDEO=4
else
	ASPARAMS	= --32
endif

LDPARAMS = -melf_i386

objects = 	obj/loader.o \
						obj/kernel/memory/gdt_stub.o \
						obj/kernel/memory/gdt.o \
						obj/kernel/memory/memorymanagement.o \
						obj/kernel/communication/ports.o \
						obj/kernel/communication/idt_stubs.o \
						obj/kernel/communication/idt.o \
						obj/kernel/multitasking.o \
						obj/kernel/drivers/vga.o \
						obj/kernel/string.o \
						obj/kernel/stdio.o \
						obj/kernel/drivers/driver.o \
						obj/kernel/drivers/keyboard.o \
						obj/kernel/drivers/mouse.o \
						obj/kernel/communication/pci.o \
						obj/kernel/drivers/AMD/am79c973.o \
						obj/kernel/network/ethernetframe.o \
						obj/kernel/drivers/vesa.o \
						obj/kernel/gui/widget.o \
						obj/kernel/gui/desktop.o \
						obj/kernel/gui/window.o \
						obj/kernel.o \

obj/%.o: src/%.cpp
	mkdir -p $(@D)
	i686-elf-g++ $(GPPPARAMS) -o $@ -c $<

obj/%.o: src/%.s
	mkdir -p $(@D)
	i686-elf-as $(ASPARAMS) -o $@ $<

kernel.elf: linker.ld $(objects)
	mkdir -p image
	ld $(LDPARAMS) -T $< -o image/$@ $(objects)

install: kernel.elf
	mkdir -p iso
	mkdir -p iso/boot
	mkdir -p iso/boot/grub
	mkdir -p release
	cp image/kernel.elf iso/boot/kernel.elf
	cp grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o os.iso iso
	rm -rf iso
	cp os.iso release/test_os.iso 

clean:
	rm -rf obj image/kernel.elf os.iso

removeFirst:
	rm -rf obj/kernel.o
	rm -rf obj/loader.o
	

run: removeFirst install
	VirtualBoxVM.exe --startvm "potatos2" &
 	#VirtualBoxVM.exe --dbg --startvm "potatos2" &
	# bochs.exe -q bochsrc.bxrc &
	#cmd.exe /c start potpot/potatos.vmx

all: clean install run 

