#include <devices.h>
#include <unicorn/unicorn.h>

DEVICE(TOPCKGEN, {
    .address = 0x10000000,
    .size = 0x1000,
});
