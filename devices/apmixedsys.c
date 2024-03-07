#include <devices.h>
#include <unicorn/unicorn.h>

const device devices_apmixedsys = {
    .address = 0x10018000,
    .size = 0x1000,
    .name = "Apmixedsys"
};
