#!/bin/bash

# RetaOS Runner Script

echo "RetaOS - Operating System Runner"
echo "================================="

# Check if build exists
if [ ! -f "build/retaos.iso" ]; then
    echo "Error: ISO file not found. Please run 'make all' first."
    exit 1
fi

# Parse command line arguments
case "$1" in
    "gfx"|"graphics")
        echo "Starting RetaOS with graphics..."
        qemu-system-i386 -cdrom build/retaos.iso -m 128M -vga std
        ;;
    "serial"|"console")
        echo "Starting RetaOS with serial output..."
        qemu-system-i386 -cdrom build/retaos.iso -m 128M -nographic
        ;;
    "debug")
        echo "Starting RetaOS in debug mode..."
        qemu-system-i386 -cdrom build/retaos.iso -m 128M -vga std -s -S
        ;;
    *)
        echo "Usage: $0 [gfx|serial|debug]"
        echo ""
        echo "Options:"
        echo "  gfx     - Run with graphics (default)"
        echo "  serial  - Run with serial console output"
        echo "  debug   - Run with debugger support"
        echo ""
        echo "Examples:"
        echo "  $0 gfx      # Run with graphics"
        echo "  $0 serial   # Run with serial output"
        echo "  $0 debug    # Run with debugger"
        exit 1
        ;;
esac 