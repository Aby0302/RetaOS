# libc Makefile
# Builds the RetaOS C library

# Cross compiler
CC = i686-linux-gnu-gcc
AR = i686-linux-gnu-ar
RANLIB = i686-linux-gnu-ranlib

# Compiler flags
CFLAGS = -m32 -std=gnu99 -ffreestanding -O2 -Wall -Wextra \
         -I./include -I../include -fno-stack-protector -fno-pie -fno-builtin -nostdinc

# Source files
SRC = src/stdio.c src/string.c src/ctype.c src/stdlib.c src/unistd.c
OBJ = $(SRC:.c=.o)
LIB = libretac.a

# Default target
all: $(LIB)

# Compile C files
%.o: %.c
	@echo "  CC    $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Create static library
$(LIB): $(OBJ)
	@echo "  AR    $@"
	@$(AR) rcs $@ $^
	@$(RANLIB) $@

# Clean
clean:
	rm -f $(OBJ) $(LIB)

.PHONY: all clean
