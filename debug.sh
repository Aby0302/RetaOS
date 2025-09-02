#!/bin/bash

# RetaOS Debug Script

# Create logs directory if it doesn't exist
mkdir -p logs

# Generate timestamp for log file
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
LOG_FILE="logs/debug_${TIMESTAMP}.log"

echo "RetaOS Debug Mode"
echo "================="
echo "Log file: $LOG_FILE"
echo "================="

# Log function
log_message() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" | tee -a "$LOG_FILE"
}

# Check if build exists
if [ ! -f "build/retaos.iso" ]; then
    log_message "ERROR: ISO file not found. Please run 'make all' first."
    exit 1
fi

if [ ! -f "build/kernel.elf" ]; then
    log_message "ERROR: kernel.elf not found. Please run 'make all' first."
    exit 1
fi

log_message "INFO: Build files found successfully"

# Check if port 1234 is available
if netstat -tlnp 2>/dev/null | grep -q ":1234 "; then
    log_message "WARNING: Port 1234 is already in use. Killing existing processes..."
    pkill qemu
    sleep 2
fi

log_message "INFO: Starting QEMU in debug mode..."
log_message "INFO: Debugger will be available on localhost:1234"
log_message "INFO: QEMU command: qemu-system-i386 -cdrom build/retaos.iso -m 128M -vga std -s -S"

echo ""
echo "Debugger will be available on localhost:1234"
echo ""
echo "To connect with GDB, open another terminal and run:"
echo "  gdb -ex 'target remote localhost:1234' -ex 'symbol-file build/kernel.elf'"
echo ""
echo "Or use the provided gdb script:"
echo "  gdb -x debug.gdb"
echo ""
echo "Log file: $LOG_FILE"
echo "Press Ctrl+C to stop QEMU"
echo ""

# Start QEMU in debug mode and log all output
log_message "INFO: Starting QEMU process..."
qemu-system-i386 -cdrom build/retaos.iso -m 128M -vga std -s 2>&1 | tee -a "$LOG_FILE" 