#include <devices.h>
#include <unicorn/unicorn.h>

const device devices_memory_sram = {
    .address = 0x10209000,
    .size = 0x1000,
    .name = "Memory.Sram"
};
