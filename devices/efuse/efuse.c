#include <devices.h>
#include <unicorn/unicorn.h>
#include <log.h>

void efuse_init(uc_engine* uc, void* devptr){
    device* dev = (device*)devptr;
    char efuse[dev->size];
    *(uint32_t*)&efuse[0] = 0x11000f80;
    efuse[0x130] = 0x40;
    uc_mem_write(uc, dev->address, &efuse, sizeof(efuse));
    DEBUG_MSG("Efuse writed!\n");
}

const device devices_efuse = {
    .address = 0x10009000,
    .size = 0x1000,
    .name = "Efuse",
    .init = efuse_init
};
