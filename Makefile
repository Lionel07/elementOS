# elementOS Build Script
# This builds elementOS, and does other things too. Try make commands.
# Based on the Makefile of Kevin Lange (https://github.com/klange)
#=================================================================
ARCH = i386-elf
ARCHDIR = x86
#CROSSCOMPILE = -target ${ARCH}
#TODO: Autodetect if using a old clang version
CROSSCOMPILE = -ccc-host-triple ${ARCH}-linux-gnu
AS := @nasm -f elf
LD := ./tool/binutils/bin/i586-elf-ld
LFLAGS := -m elf_i386
CFLAGS := -nostdlib -ffreestanding -fno-builtin -w -nostdinc -fno-stack-protector
KERNELFILES := $(patsubst %.c,%.o,$(wildcard kernel/*.c)) $(patsubst %.c,%.o,$(wildcard kernel/lib/*.c)) $(patsubst %.s,%.o,$(wildcard kernel/${ARCHDIR}/*.s)) $(patsubst %.c,%.o,$(wildcard kernel/${ARCHDIR}/*.c))
.PHONY: all clean

all: arch clean prep-fs kernel iso

rpi:
	@echo "Doing recursive make into rpi enviroment"
	@make arch clean prep-fs kernel  -e ARCH=armv6-linux-gnueabihf ARCHDIR=rpi CROSSCOMPILE="-ccc-host-triple ${ARCH} -mfloat-abi=hard"

./kernel/boot.o:
	@nasm -f elf -o ./kernel/boot.o ./kernel/${ARCHDIR}/boot.s
prep-fs:
	@cp -r ./res/${ARCHDIR}/fs/* ./fs/
kernel: ${KERNELFILES} ./kernel/boot.o
	@echo "Building Kernel..."
	@${LD} ${LFLAGS} -T kernel/${ARCHDIR}/link.ld -o kernel-${ARCHDIR}.elf ${KERNELFILES}
	@echo "Copying kernel..."
	@cp ./kernel-${ARCHDIR}.elf fs/kernel.elf

%.o: %.c
	@echo "Making: " $@
	@clang -c ${CFLAGS} ${CROSSCOMPILE} -I./kernel/includes -o $@ $<

%.o: %.s
	@echo "Making: " $@
	@nasm -f elf -o $@ $<
clean: clean-docs
	@echo "Cleaning junk..."
	@rm -R -f *.o
	@rm -R -f bootable.iso
	@rm -R -f ./kernel/*.o
	@rm -R -f ./kernel/x86/*.o
	@rm -R -f ./kernel/rpi/*.o
	@rm -R -f ./kernel/video/*.o
	@rm -R -f ./kernel/devices/*.o
	@rm -R -f ./kernel/lib/*.o
	@rm -R -f ./kernel/lib/*.o
	@rm -f kernel*.elf
	@rm -R -f ./fs/*

iso:
	@echo "Creating ISO..."
	@genisoimage -R -b boot/grub/stage2_eltorito -input-charset utf-8 -quiet -no-emul-boot -boot-load-size 4 -boot-info-table -o bootable.iso fs

run-x86:
	@echo "Running QEMU for x86"
	@-qemu-system-i386 -cdrom bootable.iso
run-rpi:
	@#echo "Running QEMU for rpi"
	@#@-qemu-system-arm -kernel kernel-qemu -cpu arm1176 -m 256 -M versatilepb -no-reboot -serial stdio -append "root=/dev/sda2 panic=1" -hda 2013-05-25-wheezy-raspbian.img
	@echo "FS is ready; copy to boot partition of sdcard..."
docs: clean-docs
	@echo -e "Cleaning Documents"
	@-doxygen Doxyfile
	@echo "Generating LaTeX documentation"
clean-docs:
	-@rm -f -r ./docs/
arch:
	@echo "Making for ${ARCH}"
