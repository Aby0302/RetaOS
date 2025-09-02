#!/bin/bash

# RetaOS Log Viewer Script

echo "RetaOS Debug Logs Viewer"
echo "======================="

# Check if logs directory exists
if [ ! -d "logs" ]; then
    echo "No logs directory found. Run debug mode first."
    exit 1
fi

# Check if there are any log files
if [ ! -f logs/debug_*.log ]; then
    echo "No debug log files found. Run debug mode first."
    exit 1
fi

echo "Available debug logs:"
echo "===================="
ls -la logs/debug_*.log 2>/dev/null | while read line; do
    echo "  $line"
done

echo ""
echo "Latest debug log:"
echo "================"

# Find the most recent log file
LATEST_LOG=$(ls -t logs/debug_*.log 2>/dev/null | head -1)

if [ -n "$LATEST_LOG" ]; then
    echo "File: $LATEST_LOG"
    echo "Size: $(du -h "$LATEST_LOG" | cut -f1)"
    echo "Last modified: $(stat -c %y "$LATEST_LOG")"
    echo ""
    echo "Content:"
    echo "========"
    cat "$LATEST_LOG"
else
    echo "No debug logs found."
fi

echo ""
echo "To view a specific log file:"
echo "  cat logs/debug_YYYYMMDD_HHMMSS.log"
echo ""
echo "To view all logs:"
echo "  find logs/ -name 'debug_*.log' -exec cat {} \;" 