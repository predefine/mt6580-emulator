#include <devices.h>
#include <unicorn/unicorn.h>

void spm_callback (uc_engine* uc, uc_mem_type type, uint64_t address, int size, int64_t value, void* user_data){
    device* dev = (device*) user_data;
    (void)dev;
    (void)uc;
    uint64_t reg = address - dev->address;
    (void)size;
    (void)value;
    if (type == UC_MEM_READ){
        if(reg == 0x60C){
            uint32_t buff = 0x1<<9;
            uc_mem_write(uc, address, &buff, sizeof(buff));
        }
    }
}

const device devices_spm = {
    .address = 0x10006000,
    .size = 0x1000,
    .callback = spm_callback,
    .name = "SPM"
};
