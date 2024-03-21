#include <devices.h>
#include <unicorn/unicorn.h>

DEVICE(Watchdog, {
    .address = 0x10007000,
    .size = 0x1000
});
