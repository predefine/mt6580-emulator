#include <devices.h>
#include <unicorn/unicorn.h>

DEVICE(PERICFG, {
    .address = 0x10003000,
    .size = 0x1000,
});
