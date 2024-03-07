#include <devices.h>
#include <unicorn/unicorn.h>

const device devices_display_pwm = {
    .address = 0x1100f000,
    .size = 0x1000,
    .name = "DisplayPWM"
};
