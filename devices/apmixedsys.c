#include <devices.h>
#include <unicorn/unicorn.h>

DEVICE(APMIXEDSYS, {
    .address = 0x10018000,
    .size = 0x1000,
});
