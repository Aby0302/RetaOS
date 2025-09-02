#include <stddef.h>
#include <stdint.h>
#include "include/drivers/serial.h"
#include "include/memory/heap.h"

#define HEAP_SIZE (64*1024)
static uint8_t heap_area[HEAP_SIZE];
static size_t heap_offset = 0;
