# Cross compiler
CC = i686-linux-gnu-gcc

# Compiler flags
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra \
         -I../include -I../libc/include -fno-stack-protector

# Linker flags
LDFLAGS = -ffreestanding -nostdlib -L../libc -lretac -lgcc

# User applications
USER_APPS = crt/init.elf sh/shell.elf crt/hello.elf test/fat32_test.elf crt/keyboard_utils.elf

# All source files
SOURCES = $(wildcard crt/*.c sh/*.c test/*.c) crt/keyboard_utils.c
OBJECTS = $(SOURCES:.c=.o)

# Default target
all: libc $(USER_APPS)

# Build libc first
libc:
	$(MAKE) -C ../libc

# Clean libc
clean-libc:
	$(MAKE) -C ../libc clean

# Compilation rule
%.o: %.c
	@echo "Compiling $<..."
	@mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS)

# Linking rule
%.elf: %.o
	@echo "Linking $@..."
	@mkdir -p $(@D)
	$(CC) -T ../boot/linker.ld -o $@ $< $(LDFLAGS)

# Clean rule
clean:
	rm -f $(OBJECTS) $(USER_APPS)

# Dependencies
crt/init.o: crt/init.c
sh/shell.o: sh/shell.c
crt/hello.o: crt/hello.c
test/fat32_test.o: test/fat32_test.c
