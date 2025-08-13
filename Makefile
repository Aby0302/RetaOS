# RetaOS Makefile
# Builds a minimal multiboot kernel

# Compiler and tools
CC = gcc
AS = nasm
LD = ld
OBJCOPY = objcopy

# Compiler flags for bare metal
CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -fno-builtin -fno-pic -mno-red-zone -Wall -Wextra -std=c99
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld

# Directories
BUILD_DIR = build
SRC_DIR = .

# Source files
KERNEL_SOURCES = kernel.c gdt.c idt.c isr.c serial.c
ASM_NASM_SOURCES = boot.s
ASM_GAS_SOURCES = gdt_flush.S idt_load.S
OBJECTS = $(KERNEL_SOURCES:.c=.o) $(ASM_NASM_SOURCES:.s=.o) $(ASM_GAS_SOURCES:.S=.o) isr_stub.o

# Target files
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
ISO_FILE = $(BUILD_DIR)/retaos.iso

# Default target
all: $(ISO_FILE)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble NASM assembly files (.s)
%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

# Assemble GAS assembly files (.S)
%.o: %.S
	$(CC) -m32 -c $< -o $@

# Special rule: build ISR stubs with a unique object name
isr_stub.o: isr.S
	$(CC) -m32 -c $< -o $@

# Link kernel
$(KERNEL_ELF): $(OBJECTS) | $(BUILD_DIR)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

# Create binary from ELF
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# Create ISO
$(ISO_FILE): $(KERNEL_ELF)
	mkdir -p $(BUILD_DIR)/iso/boot/grub
	cp $(KERNEL_ELF) $(BUILD_DIR)/iso/boot/
	echo 'menuentry "RetaOS" {' > $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '    multiboot /boot/kernel.elf' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '    boot' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '}' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(BUILD_DIR)/iso

# Run in QEMU
run: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE) -m 128M -nographic

# Run with graphics
run-gfx: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE) -m 128M -vga std

# Run with curses display (VGA text in terminal)
run-curses: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE) -m 128M -display curses

# Clean build files
clean:
	rm -rf $(BUILD_DIR)
	rm -f *.o

# Install GRUB tools if needed
install-grub:
	sudo apt install -y grub-pc-bin

# Show help
help:
	@echo "Available targets:"
	@echo "  all        - Build the complete ISO"
	@echo "  run        - Run in QEMU with serial output (no VGA output)"
	@echo "  run-gfx    - Run in QEMU with graphics (shows VGA text)"
	@echo "  run-curses - Run in QEMU with curses display in terminal"
	@echo "  clean      - Clean build files"
	@echo "  install-grub - Install GRUB tools"

.PHONY: all run run-gfx run-curses clean install-grub help
