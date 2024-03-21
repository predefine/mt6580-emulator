#include <devices.h>
#include <stdlib.h>
#include <string.h>

static devices_list* devices;

static void __attribute__((constructor(DEVICES_INIT_PRIOR))) init_devices(){
    devices = NULL;
}

const devices_list* get_devices(){
    return devices;
}

void add_device(char* name, device _dev){
    devices_list* devlist = malloc(sizeof(devices_list));
    devlist->this = malloc(sizeof(device));
    memcpy(devlist->this, &_dev, sizeof(device));
    strncpy(devlist->this->name, name, 128);
    devlist->next = devices;
    devices = devlist;
}

DEVICE(__NONREAL__UBOOT_BUG1, {
    .address = 0x8000000,
    .size = 0x1000
});

DEVICE(__NONREAL__UBOOT_RANDOM1, {
    .address = 0x7e198000,
    .size = 0x1000
});
