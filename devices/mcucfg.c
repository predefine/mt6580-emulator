#include <devices.h>
#include <unicorn/unicorn.h>

void mcucfg_init(uc_engine* uc, void* devptr){
    device* dev = (device*)devptr;
    uint32_t reg0;
    reg0 = 0b1100000;
    uc_mem_write(uc, dev->address, &reg0, sizeof(reg0));
}


DEVICE(MCUCFG, {
    .address = 0x10200000,
    .size = 0x1000,
    .init = mcucfg_init,
});
