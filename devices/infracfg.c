#include <devices.h>
#include <unicorn/unicorn.h>

const device devices_infracfg_ao = {
    .address = 0x10001000,
    .size = 0x1000,
    .name = "Infracfg.AO"
};
