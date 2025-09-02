# RetaOS Cross-Platform Makefile

# Platform-agnostic LLVM setup
LLVM_PREFIX := /usr

# Compiler and tools
CC = /usr/bin/clang
AS = nasm
LD := $(shell command -v ld.lld 2>/dev/null || command -v lld 2>/dev/null || command -v $(LLVM_PREFIX)/bin/lld 2>/dev/null || command -v ld 2>/dev/null)
OBJCOPY = $(LLVM_PREFIX)/bin/llvm-objcopy

# Simplified GRUB detection (uses xorriso as fallback)
MKRESCUE := $(shell command -v xorriso 2>/dev/null)
HAS_GRUB := $(MKRESCUE)

# Standardized compiler flags
CFLAGS = -m32 -Wall -Wextra -std=c99 -g -I libc/include -I . -I include -I include/arch/x86 -I include/kernel
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T boot/linker.ld

# Source files
C_SOURCES = $(filter-out kernel/keyboard_utils.c fs/fat32_impl.c fs/fat32_new.c,$(wildcard kernel/*.c memory/*.c arch/x86/x86/*.c arch/x86/x86/interrupts/*.c arch/x86/x86/memory/*.c arch/x86/x86/gdt/*.c drivers/input/*.c drivers/keyboard/*.c drivers/serial/*.c fs/*.c))
ASM_GAS_SOURCES = $(filter-out arch/x86/x86/interrupts/isr.S, $(wildcard arch/x86/x86/boot/*.S arch/x86/x86/*.S arch/x86/x86/interrupts/*.S arch/x86/x86/gdt/*.S))

# Objects
OBJ_C = $(patsubst %.c,build/obj/%.o,$(C_SOURCES))
OBJ_GAS = $(patsubst %.S,build/obj/%.o,$(ASM_GAS_SOURCES))
OBJECTS = $(OBJ_C) $(OBJ_GAS)

# Target files
BUILD_DIR = build
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
ISO_FILE = $(BUILD_DIR)/retaos.iso

# Default target: build kernel only for linux
all: $(KERNEL_ELF)
	make clean
	@echo "Built $(KERNEL_ELF)."
	make run-gfx

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile C files
build/obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble GAS files
build/obj/%.o: %.S
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -o $@ $<

# Link kernel
$(KERNEL_ELF): $(OBJECTS) | $(BUILD_DIR)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

# Create binary from ELF
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD_DIR)
	clear
