#include <devices.h>
#include <unicorn/unicorn.h>

void security_asfv2_callback (uc_engine* uc, uc_mem_type type, uint64_t address, int size, long value, void* user_data){
    device* dev = (device*) user_data;
    (void)dev;
    (void)uc;
    uint64_t reg = address - dev->address;
    (void)size;
    (void)value;
    if (type == UC_MEM_READ){
        if(reg == 8){
            // im think bit 15 is "is ready"
            uint32_t buff = 1<<15;
            uc_mem_write(uc, address, &buff, sizeof(buff));
        }
    }
}

DEVICE(SECURITY_ASFV2, {
    .address = 0x1000a000,
    .size = 0x1000,
    .callback = security_asfv2_callback,
});
