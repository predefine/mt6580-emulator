#include <devices.h>
#include <unicorn/unicorn.h>
DEVICE(USB_phy, {
    .address = 0x11110000,
    .size = 0x1000,
});

DEVICE(USB0, {
    .address = 0x11100000,
    .size = 0x1000,
});
