# Cross compiler
CC = i686-linux-gnu-gcc

# Compiler flags
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra \
         -I../include -I../libc/include -fno-stack-protector

# Linker flags (no kernel linker script; user programs have their own _start)
LDFLAGS = -ffreestanding -nostdlib -L../libc -lretac -lgcc

# CRT start object
CRT_START = crt/start.o

# User applications
USER_APPS = crt/init.elf sh/shell.elf crt/hello.elf gui/gui.elf

# All source files
SOURCES = $(wildcard crt/*.c sh/*.c gui/*.c)
OBJECTS = $(SOURCES:.c=.o)

# Default target
all: ../libc/libretac.a $(USER_APPS)

# Build libc first (use file dependency to avoid parallel races)
../libc/libretac.a:
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
%.elf: %.o $(CRT_START) ../libc/libretac.a
	@echo "Linking $@..."
	@mkdir -p $(@D)
	$(CC) -o $@ $(CRT_START) $< $(LDFLAGS) -no-pie -Wl,-Ttext=0x00100000

# Clean rule
clean:
	rm -f $(OBJECTS) $(USER_APPS)

# Dependencies
crt/init.o: crt/init.c
sh/shell.o: sh/shell.c
crt/hello.o: crt/hello.c
gui/gui.o: gui/gui.c
test/fat32_test.o: test/fat32_test.c
