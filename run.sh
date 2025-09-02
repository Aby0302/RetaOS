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
        echo "Serial output will be written to /tmp/qemu-serial.log"
        # Remove any existing log file
        rm -f /tmp/qemu-serial.log
        # Start QEMU with serial output to file and console
        qemu-system-i386 -cdrom build/retaos.iso -m 128M \
            -serial stdio \
            -serial file:/tmp/qemu-serial.log \
            -monitor none \
            -nographic
        ;;
    "debug")
        # Create logs directory if it doesn't exist
        mkdir -p logs
        
        # Generate timestamp for log file
        TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
        LOG_FILE="logs/debug_${TIMESTAMP}.log"
        
        echo "Starting RetaOS in debug mode..."
        echo "Log file: $LOG_FILE"
        echo "Debugger will be available on localhost:1234"
        echo "To connect with GDB: gdb -ex 'target remote localhost:1234' -ex 'symbol-file build/kernel.elf'"
        echo "Press Ctrl+C to stop the debugger"
        echo ""
        
        # Log function
        log_message() {
            echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" | tee -a "$LOG_FILE"
        }
        
        log_message "INFO: Starting QEMU in debug mode via run.sh"
        log_message "INFO: QEMU command: qemu-system-i386 -cdrom build/retaos.iso -m 128M -vga std -s -S"
        
        qemu-system-i386 -cdrom build/retaos.iso -m 128M -vga std -s 2>&1 | tee -a "$LOG_FILE"
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