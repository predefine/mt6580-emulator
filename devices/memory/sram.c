#include <devices.h>
#include <unicorn/unicorn.h>

DEVICE(MEMORY_SRAM, {
    .address = 0x10209000,
    .size = 0x1000,
});
