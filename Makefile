# RetaOS Cross-Platform Makefile

# Platform-agnostic LLVM setup
LLVM_PREFIX := /usr

# Compiler and tools
CC = /usr/bin/clang
AS = nasm
LD := $(shell command -v ld.lld 2>/dev/null || command -v lld 2>/dev/null || command -v $(LLVM_PREFIX)/bin/lld 2>/dev/null || command -v ld 2>/dev/null)
OBJCOPY = $(LLVM_PREFIX)/bin/llvm-objcopy

# Tool detection
GRUB_MKRESCUE := $(shell command -v grub-mkrescue 2>/dev/null)

# Standardized compiler flags
CFLAGS = -m32 -Wall -Wextra -std=c99 -g -I libc/include -I . -I include -I include/arch/x86 -I include/kernel
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T boot/linker.ld

# Correct the path to libc library
LDLIBS = libc/libretac.a

# Source files
C_SOURCES = $(filter-out kernel/keyboard_utils.c fs/fat32_impl.c fs/fat32_new.c,$(wildcard kernel/*.c memory/*.c arch/x86/x86/*.c arch/x86/x86/interrupts/*.c arch/x86/x86/memory/*.c arch/x86/x86/gdt/*.c drivers/input/*.c drivers/keyboard/*.c drivers/serial/*.c drivers/ata/*.c fs/*.c))
# Adding sound and network drivers to the build
C_SOURCES += $(wildcard drivers/sound/*.c drivers/network/*.c)
ASM_GAS_SOURCES = $(filter-out arch/x86/x86/interrupts/isr.S, $(wildcard arch/x86/x86/boot/*.S arch/x86/x86/*.S arch/x86/x86/interrupts/*.S arch/x86/x86/gdt/*.S))
# GAS-style assembly source (assemble with clang/gcc, not NASM)
ISR_STUBS_SRC = arch/x86/x86/interrupts/isr.S

# Objects
OBJ_C = $(patsubst %.c,build/obj/%.o,$(C_SOURCES))
OBJ_GAS = $(patsubst %.S,build/obj/%.o,$(ASM_GAS_SOURCES))
OBJECTS = $(OBJ_C) $(OBJ_GAS) build/obj/arch/x86/x86/interrupts/isr_stubs.o

# Target files
BUILD_DIR = build
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
ISO_FILE = $(BUILD_DIR)/retaos.iso
ISO_DIR = $(BUILD_DIR)/isodir
GRUB_DIR = $(ISO_DIR)/boot/grub
KERNEL_ISO_PATH = $(ISO_DIR)/boot/kernel.elf
DISK_IMG = $(BUILD_DIR)/disk.img

# Default target: build everything (kernel+ISO, libc+user apps, initrd, disk)
all: $(ISO_FILE) disk
	@echo "\n=== Build Summary ==="
	@echo "Kernel ELF : $(KERNEL_ELF)"
	@echo "ISO        : $(ISO_FILE)"
	@echo "Disk image : $(DISK_IMG) (initrd at LBA 1)"
	@echo "======================\n"

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

# Assemble GAS-style ISR stubs with clang (GNU assembler syntax)
build/obj/arch/x86/x86/interrupts/isr_stubs.o: $(ISR_STUBS_SRC)
	@mkdir -p $(dir $@)
	$(CC) -m32 -c $< -o $@

# Link kernel
$(KERNEL_ELF): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) libc/libretac.a

# Create binary from ELF
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# Build GRUB ISO around kernel (ensure initrd.tar exists)
$(ISO_FILE): $(KERNEL_ELF) initrd
	@mkdir -p $(GRUB_DIR)
	@cp $(KERNEL_ELF) $(KERNEL_ISO_PATH)
	@cp initrd.tar $(ISO_DIR)/boot/initrd.tar
	@cp -f initrd.cpio $(ISO_DIR)/boot/initrd.cpio 2>/dev/null || true
	@printf 'set timeout=0\nset default=0\n\nmenuentry "RetaOS" {\n  multiboot /boot/kernel.elf\n  module2   /boot/initrd.tar initrd\n  boot\n}\n' > $(GRUB_DIR)/grub.cfg
	@if [ -z "$(GRUB_MKRESCUE)" ]; then \
	  echo "Error: grub-mkrescue not found. Please install grub-pc-bin and xorriso."; \
	  exit 1; \
	fi
	$(GRUB_MKRESCUE) -o $(ISO_FILE) $(ISO_DIR)

.PHONY: run-gfx run debug

# Run in QEMU (graphics)
run-gfx: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE) -m 128M -vga std

# Run in QEMU (serial/headless)
run: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE) -m 128M -serial stdio -display none

# Debug via helper script (ensures ISO exists)
debug: $(ISO_FILE)
	./debug.sh

# Build user applications
.PHONY: user
user:
	$(MAKE) -C user all

# Package initrd from initroot and optional user apps
.PHONY: initrd
initrd: user
	@cp -f user/sh/shell.elf initroot/bin/sh 2>/dev/null || true
	@cp -f user/crt/init.elf initroot/bin/init.elf 2>/dev/null || true
	@cp -f user/gui/gui.elf initroot/bin/gui 2>/dev/null || true
	tar --format=ustar -C initroot -cf initrd.tar .

# Create initrd.cpio using cpio from rootfs
.PHONY: initrd-cpio
initrd-cpio:
	(cd rootfs && find . | cpio -H newc -o) > initrd.cpio

# Create a simple ATA disk with initrd at LBA 1
.PHONY: disk
disk: initrd | $(BUILD_DIR)
	dd if=/dev/zero of=$(DISK_IMG) bs=1M count=16 status=none
	dd if=initrd.tar of=$(DISK_IMG) bs=512 seek=1 conv=notrunc status=none
	@echo "Created $(DISK_IMG) with initrd.tar at LBA 1"

.PHONY: run-gfx-disk run-disk

run-gfx-disk: $(ISO_FILE) disk
	qemu-system-i386 -cdrom $(ISO_FILE) -drive file=$(DISK_IMG),format=raw,if=ide,index=0,media=disk -m 128M -vga std

run-disk: $(ISO_FILE) disk
	qemu-system-i386 -cdrom $(ISO_FILE) -drive file=$(DISK_IMG),format=raw,if=ide,index=0,media=disk -m 128M -serial stdio -display none

clean:
	rm -rf $(BUILD_DIR)
	clear

.PHONY: debug-trace
debug-trace: $(ISO_FILE)
	@mkdir -p logs
	qemu-system-i386 -cdrom $(ISO_FILE) -m 128M -serial stdio -display none -d int,guest_errors,cpu_reset -D logs/qemu_trace.log -no-reboot -no-shutdown
