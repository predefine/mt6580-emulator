#include <devices.h>
#include <unicorn/unicorn.h>

DEVICE(DBGSYS, {
    .address = 0x1011a000,
    .size = 0x1000,
});
