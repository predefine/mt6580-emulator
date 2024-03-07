#include <devices.h>
#include <unicorn/unicorn.h>

const device devices_dbgsys = {
    .address = 0x1011a000,
    .size = 0x1000,
    .name = "Dbgsys"
};
