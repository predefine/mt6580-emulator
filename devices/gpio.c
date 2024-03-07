#include <devices.h>
#include <unicorn/unicorn.h>
#include <log.h>

void devices_gpio_hook (uc_engine* uc, uc_mem_type type, uint64_t address, int size, long value, void* user_data){
    device* dev = (device*) user_data;
    (void)dev;
    (void)uc;
    uint64_t reg = address - dev->address;
    (void)size;
    (void)value;
    (void)reg;
    if(reg == 0x144 && type == UC_MEM_READ){
        uint32_t value2 = 0;
        value |= 0b1000001;
        value |= (value << 8);
        uc_mem_write(uc, address, &value2, sizeof(value2));
    } else
        DEBUG_MSG("[%s] address 0x%lx was %s!\n",dev->name, address, type == UC_MEM_READ ? "readed":"writed");
}


const device devices_gpio = {
    .address = 0x10005000,
    .size = 0x1000,
    .callback = devices_gpio_hook,
    .name = "Gpio"
};
const device devices_gpio_bottom = {
    .address = 0x10015000,
    .size = 0x1000,
    .callback = devices_gpio_hook,
    .name = "Gpio.Bottom"
};

const device devices_gpio_right = {
    .address = 0x10017000,
    .size = 0x1000,
    .callback = devices_gpio_hook,
    .name = "Gpio.Right"
};

