#include <devices.h>
#include <stdlib.h>

extern device devices_timers_gpt;
extern device devices_serial_uart0;
extern device devices_serial_uart1;
extern device devices_efuse;
extern device devices_security_asfv2;
extern device devices_gpio;
extern device devices_gpio_bottom;
extern device devices_gpio_right;
extern device devices_memory_sram;
extern device devices_apmixedsys;
extern device devices_mcucfg;
extern device devices_msdc_emmc;
extern device devices_msdc_sdcard;
extern device devices_topckgen;
extern device devices_wdt;
extern device devices_pericfg;
extern device devices_dbgsys;
extern device devices_infracfg_ao;
extern device devices_usbphy;
extern device devices_usb0;
extern device devices_display_pwm;
extern device devices_spm;

device devices_uboot_bug_address_1 = {
    .address = 0x8000000,
    .size = 0x1000,
    .name = "Uboot bug address 1"
};

// device devices_uboot_random_address_1 = {
//     .address = 0xa0000000,
//     .size = 0x1000,
//     .name = "Uboot random address 1"
// };

device devices_uboot_random_address_2 = {
    .address = 0x7e198000,
    .size = 0x1000,
    .name = "Uboot random address 2"
};

// device devices_dramc = {
//     .address = 0xc0000000,
//     .size = 0x1000,
//     .name = "Dram controller"
// };

device *devices[] = {
    &devices_timers_gpt,
    &devices_serial_uart0,
    &devices_serial_uart1,
    &devices_efuse,
    &devices_security_asfv2,
    &devices_memory_sram,
    &devices_gpio,
    &devices_gpio_bottom,
    &devices_gpio_right,
    &devices_apmixedsys,
    &devices_mcucfg,
    &devices_msdc_emmc,
    &devices_msdc_sdcard,
    &devices_topckgen,
    &devices_wdt,
    &devices_pericfg,
    &devices_infracfg_ao,
    &devices_uboot_random_address_2,
    &devices_uboot_bug_address_1,
    &devices_dbgsys,
    &devices_usbphy,
    &devices_usb0,
    &devices_display_pwm,
    &devices_spm,
    NULL
};
