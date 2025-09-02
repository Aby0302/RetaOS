# RetaOS GDB Debug Script
# This script automatically connects to QEMU and loads kernel symbols

# Disable pagination
set pagination off

# Connect to QEMU debugger
target remote localhost:1234

# Load kernel symbols
symbol-file build/kernel.elf

# Set architecture
set architecture i386

# Set some useful breakpoints
break kernel_main
break kmain

# Show current status
info registers
info breakpoints

# Print welcome message
echo 
echo ========================================
echo RetaOS Debugger Connected Successfully!
echo ========================================
echo 
echo Available commands:
echo   continue (c)     - Continue execution
echo   step (s)         - Step into function
echo   next (n)         - Step over function
echo   break function   - Set breakpoint at function
echo   info registers   - Show CPU registers
echo   x/10i $eip      - Show 10 instructions at current position
echo   bt               - Show backtrace
echo 
echo Type 'help' for more commands
echo 
