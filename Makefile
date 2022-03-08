
GPPPARAMS = -s -m32 -Iinclude -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings -fpermissive -fno-pic -ffreestanding -Wl,--build-id=none
ASPARAMS	= --32
LDPARAMS = -melf_i386

objects = 	obj/loader.o \
						obj/memory/gdt_stub.o \
						obj/memory/gdt.o \
						obj/common/hardware/vga.o \
						obj/common/string.o \
						obj/common/stdio.o \
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
	mkdir -p testbuild
	cp image/kernel.elf iso/boot/kernel.elf
	cp grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o os.iso iso
	rm -rf iso
	cp os.iso testbuild/test_os.iso 

clean:
	rm -rf obj image/kernel.elf os.iso

run: install
	#VirtualBoxVM.exe --startvm "potatos" &
	#VirtualBoxVM.exe --dbg --startvm "potatos" &
	#bochs.exe -q bochsrc.bxrc &
	cmd.exe /c start potpot/potatos.vmx

all: clean install run 

