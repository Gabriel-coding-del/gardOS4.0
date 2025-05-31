# Copyright (C) 2025 Gabriel Sîrbu
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

NASM = nasm
CC = i686-elf-gcc
LD = i686-elf-ld

# Directories for ISO structure
ISO_DIR = iso
GRUB_DIR = $(ISO_DIR)/boot/grub

all: kernel.bin iso

# Assemble boot.asm into an ELF32 object file
boot.o: boot.asm
	$(NASM) -f elf32 boot.asm -o boot.o

# Compile kernel.c into an object file for 32-bit
kernel.o: kernel.c
	$(CC) -ffreestanding -m32 -c kernel.c -o kernel.o

# Link boot.o and kernel.o into a multiboot kernel binary
kernel.bin: boot.o kernel.o link.ld
	$(LD) -m elf_i386 -T link.ld -o kernel.bin boot.o kernel.o

# Create the ISO image using GRUB
iso: kernel.bin
	mkdir -p $(GRUB_DIR)
	cp kernel.bin $(ISO_DIR)/boot/
	@echo "set timeout=0" > $(GRUB_DIR)/grub.cfg
	@echo "menuentry 'My OS' {" >> $(GRUB_DIR)/grub.cfg
	@echo "    multiboot /boot/kernel.bin quiet" >> $(GRUB_DIR)/grub.cfg
	@echo "    boot" >> $(GRUB_DIR)/grub.cfg
	@echo "}" >> $(GRUB_DIR)/grub.cfg
	grub-mkrescue -o gardOS.iso $(ISO_DIR)

# Run the OS in QEMU
run: all
	qemu-system-i386 -cdrom gardOS.iso

clean:
	rm -f *.o kernel.bin
	rm -rf $(ISO_DIR) gardOS.iso
