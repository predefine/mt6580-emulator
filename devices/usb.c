#include <devices.h>
#include <unicorn/unicorn.h>

const device devices_usbphy = {
    .address = 0x11110000,
    .size = 0x1000,
    .name = "USB phy"
};
const device devices_usb0 = {
    .address = 0x11100000,
    .size = 0x1000,
    .name = "USB0"
};
