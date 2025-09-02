#!/bin/bash

# Create necessary directories
mkdir -p arch/x86/{memory,gdt,interrupts}
mkdir -p drivers/{serial,keyboard,vga,ata,timer}
mkdir -p fs
mkdir -p lib
mkdir -p include/{kernel,arch/x86,drivers,memory,fs,lib}
mkdir -p user/{crt,sh,echo,cat}
mkdir -p initroot/{bin,etc,dev}
mkdir -p scripts configs tests tools third_party

# Move kernel files
mv kernel/*.c kernel/
mv kernel/*.h include/kernel/

# Move architecture files
mv arch/x86/* arch/x86/
mv include/arch/x86/* include/arch/x86/

# Move driver files
mv drivers/* drivers/
mv include/drivers/* include/drivers/

# Move memory management files
mv memory/*.c memory/
mv memory/*.h include/memory/

# Move lib files
mv lib/*.c lib/
mv lib/*.h include/lib/

# Move user programs
mv user/hello.c user/crt/
mv user/init.c user/crt/
mv user/shell.c user/sh/

# Move boot files
mv boot2.S boot/
mv linker.ld boot/

# Clean up empty directories
find . -type d -empty -delete

# Update Makefile paths (already done manually)

echo "Project structure reorganized successfully!"
