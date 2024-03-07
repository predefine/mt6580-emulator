#include <devices.h>
#include <unicorn/unicorn.h>

const device devices_topckgen = {
    .address = 0x10000000,
    .size = 0x1000,
    .name = "Topckgen"
};
