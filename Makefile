# elementOS Build Script
# This builds elementOS, and does other things too. Try make commands.
# Based on the Makefile of Kevin Lange (https://github.com/klange)
#=================================================================
#Setup
ARCH := i386-elf-linux-gnu
CFLAGS :=
OPTIONS := -D ENABLE_DEBUG #-D OPT_NO_PROGRESS_BARS
DEBUG :=
AS := @nasm -f elf
ASM := nasm
AFLAGS := -f elf

LD := ./tool/binutils/bin/i586-elf-ld
LFLAGS := -m elf_i386

#FILES
KERNELFILES := $(patsubst %.c,%.o,$(wildcard kernel/*.c)) $(patsubst %.c,%.o,$(wildcard kernel/lib/*.c)) $(patsubst %.c,%.o,$(wildcard kernel/video/*.c)) $(patsubst %.s,%.o,$(wildcard kernel/x86/*.s)) $(patsubst %.c,%.o,$(wildcard kernel/x86/*.c) ) $(patsubst %.c,%.o,$(wildcard kernel/devices/*.c))
#Rules
.PHONY: all clean
all: configure system install mkmedia dist
aux: docs
system: arch kernel

install:
	@echo "Installing kernel into filesystem..."
	@cp ./kernel.elf fs/kernel.elf

kernel: clean kernel/boot.o ${KERNELFILES}
	@echo "Building Kernel..."
	@${LD} ${LFLAGS} -T kernel/x86/link.ld -o kernel.elf ${KERNELFILES}

%.o: %.c
	@echo "Making: " $@
	@clang -c -O3 -w -ffreestanding -fno-builtin  -nostdlib -nostdinc -fno-stack-protector ${OPTIONS}  -ccc-host-triple i586-elf-linux-gnu -I./kernel/includes -o $@ $<

%.o: %.s
	@echo "Making: " $@
	@nasm -f elf -o $@ $<
kernel/boot.o: kernel/x86/boot.s
	@echo "Making: " $@
	@${AS} -o kernel/boot.o kernel/x86/boot.s
clean: clean-docs
	@echo "Cleaning junk..."
	@rm -R -f *.o
	@rm -R -f ./kernel/*.o
	@rm -R -f ./kernel/x86/*.o
	@rm -R -f ./kernel/video/*.o
	@rm -R -f ./kernel/devices/*.o
	@rm -R -f ./kernel/lib/*.o

	@rm -f kernel.elf

mkmedia: mkmedia-iso
mkmedia-iso:
	@echo "Creating ISO..."
	@genisoimage -R -b boot/grub/stage2_eltorito -input-charset utf-8 -quiet -no-emul-boot -boot-load-size 4 -boot-info-table -o bootable.iso fs

configure:

run:
	@echo "Running QEMU"
	@-qemu-system-i386 -cdrom bootable.iso
docs: clean-docs
	@echo -e "Cleaning Documents"
	@-doxygen Doxyfile
	@echo "Generating LaTeX documentation"
clean-docs:
	-@rm -f -r /docs/
ready-dist:
	@echo "Preparing for tarball"
	@rm -R -f *.o
	@rm -R -f ./kernel/*.o
	@rm -R -f ./kernel/arch/*.o
	@rm -R -f ./kernel/video/*.o
	@rm -R -f ./kernel/devices/*.o
	@rm -R -f ./kernel/lib/*.o
dist: ready-dist
	@echo "Making Distrobution / tarball"
	@tar --exclude=elementOS-dist.tar --exclude binutils* -cf elementOS-dist.tar --add-file ./
arch:
	@echo "Making for ${ARCH}"
