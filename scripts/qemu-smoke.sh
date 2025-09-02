#!/usr/bin/env bash
set -euo pipefail

# Simple headless QEMU smoke test for RetaOS
# Requirements: qemu-system-i386, grub-mkrescue, xorriso

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
BUILD_DIR="$ROOT_DIR/build"
ISO_FILE="$BUILD_DIR/retaos.iso"

# Build if needed
make -C "$ROOT_DIR" -j"$(nproc)" all

# Run QEMU headless and capture output with timeout
# Success criterion: kernel prints the welcome banner
SUCCESS_PATTERN="Welcome to RetaOS!"

# Some CI environments need -no-reboot -no-shutdown to let QEMU exit on its own
# We use timeout to ensure we don't hang indefinitely
set +e
OUTPUT=$(timeout 30s qemu-system-i386 -cdrom "$ISO_FILE" -m 128M -nographic -no-reboot -no-shutdown 2>&1)
STATUS=$?
set -e

# Echo output to aid debugging
echo "$OUTPUT"

if [[ $STATUS -ne 0 ]]; then
  echo "[smoke] QEMU exited with status $STATUS" >&2
  exit $STATUS
fi

if echo "$OUTPUT" | grep -q "$SUCCESS_PATTERN"; then
  echo "[smoke] Boot successful (pattern found)."
  exit 0
else
  echo "[smoke] Boot banner not found: '$SUCCESS_PATTERN'" >&2
  exit 1
fi
