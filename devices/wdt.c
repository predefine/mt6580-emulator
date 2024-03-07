#include <devices.h>
#include <unicorn/unicorn.h>

const device devices_wdt = {
    .address = 0x10007000,
    .size = 0x1000,
    .name = "Watchdog"
};
