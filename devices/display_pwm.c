#include <devices.h>
#include <unicorn/unicorn.h>

DEVICE(DISPLAY_PWM, {
    .address = 0x1100f000,
    .size = 0x1000,
});
