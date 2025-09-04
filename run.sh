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
        # Attach disk image if present so ATA initrd mount can work
        if [ -f "build/disk.img" ]; then
            qemu-system-i386 -cdrom build/retaos.iso -drive file=build/disk.img,format=raw,if=ide,index=0,media=disk -m 128M -vga std
        else
            echo "NOTE: build/disk.img not found; running without ATA disk."
            echo "      Run 'make disk' or 'make all' to create it."
            qemu-system-i386 -cdrom build/retaos.iso -m 128M -vga std
        fi
        ;;
    "serial"|"console")
        echo "Starting RetaOS with serial output..."
        # Create logs directory and pick a timestamped logfile
        mkdir -p logs
        TS=$(date +"%d_%m_%Y_%H:%M:%S")
        LOG_FILE="logs/serial_${TS}.log"
        echo "Serial output will be logged to $LOG_FILE"
        # Use a stdio chardev with logfile so output goes to both terminal and file
        if [ -f "build/disk.img" ]; then
            DRIVE_ARGS="-drive file=build/disk.img,format=raw,if=ide,index=0,media=disk"
        else
            echo "NOTE: build/disk.img not found; running without ATA disk."
            echo "      Run 'make disk' or 'make all' to create it."
            DRIVE_ARGS=""
        fi
        qemu-system-i386 -cdrom build/retaos.iso ${DRIVE_ARGS} -m 128M \
            -chardev stdio,id=char0,logfile="${LOG_FILE}",logappend=on \
            -serial chardev:char0 \
            -monitor none \
            -display none
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
