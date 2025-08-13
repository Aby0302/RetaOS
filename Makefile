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
SRC_DIR = src

# Source files
C_SOURCES = \
	src/kernel/kernel.c \
	src/kernel/multiboot.c \
	src/memory/pmm.c \
	src/memory/heap.c \
	src/arch/x86/memory/paging.c \
	src/arch/x86/gdt/gdt.c \
	src/arch/x86/interrupts/idt.c \
	src/arch/x86/interrupts/isr.c \
	src/drivers/serial/serial.c \
	src/drivers/keyboard/keyboard.c

ASM_NASM_SOURCES = \
	src/arch/x86/boot/boot.s

ASM_GAS_SOURCES = \
	src/arch/x86/boot/boot2.S \
	src/arch/x86/gdt/gdt_flush.S \
	src/arch/x86/interrupts/idt_load.S

# Objects
OBJ_C   = $(patsubst %.c,$(BUILD_DIR)/obj/%.o,$(C_SOURCES))
OBJ_NAS = $(patsubst %.s,$(BUILD_DIR)/obj/%.o,$(ASM_NASM_SOURCES))
OBJ_GAS = $(patsubst %.S,$(BUILD_DIR)/obj/%.o,$(ASM_GAS_SOURCES))
ISR_STUB_OBJ = $(BUILD_DIR)/obj/src/arch/x86/interrupts/isr_stub.o
OBJECTS = $(OBJ_C) $(OBJ_NAS) $(OBJ_GAS) $(ISR_STUB_OBJ)

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
$(BUILD_DIR)/obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I . -c $< -o $@

# Assemble NASM assembly files (.s)
$(BUILD_DIR)/obj/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# Assemble GAS assembly files (.S)
$(BUILD_DIR)/obj/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) -m32 -c $< -o $@

# Special rule: build ISR stubs with a unique object name
$(ISR_STUB_OBJ): src/arch/x86/interrupts/isr.S
	@mkdir -p $(dir $@)
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
	grub-mkrescue -o $@ $(BUILD_DIR)/iso | cat

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
