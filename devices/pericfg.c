#include <devices.h>
#include <unicorn/unicorn.h>

const device devices_pericfg = {
    .address = 0x10003000,
    .size = 0x1000,
    .name = "Pericfg"
};
