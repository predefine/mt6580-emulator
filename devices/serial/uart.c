#include <devices.h>
#include <stdio.h>
#include <termios.h>
#include <unicorn/unicorn.h>
#include <fcntl.h>

void init_terminal()
{
    fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
    struct termios term;
    tcgetattr(0, &term);
    term.c_iflag &= ~ECHO;
    term.c_iflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &term);
}

void serial_uart_init(uc_engine* uc, void* devptr){
    (void)uc;
    (void)devptr;
    init_terminal();
}

void serial_uart_callback (uc_engine* uc, uc_mem_type type, uint64_t address, int size, int64_t value, void* user_data){
    device* dev = (device*) user_data;
    (void)dev;
    (void)uc;
    uint64_t reg = address - dev->address;
    (void)size;
    // 0x0 = rx and tx buffer
    // 0x14 latest bit = data ready
    if(type == UC_MEM_WRITE){
        if(reg == 0 && value != 0){
            putchar((char)value);
        }
    } else if(type == UC_MEM_READ) {
        if(reg == 0x14){
            uint8_t temp = -1;
            uc_mem_write(uc, address, &temp, sizeof(uint32_t));
        } else if(reg == 0x0){
            uint8_t to_write = 0;
            to_write = getchar();
            uc_mem_write(uc, address, &to_write, sizeof(uint32_t));

        }
    }
}

DEVICE(SERIAL_UART0, {
    .address = 0x11005000,
    .size = 0x1000,
    .callback = serial_uart_callback,
    .init = serial_uart_init,
});

DEVICE(SERIAL_UART1, {
    .address = 0x11006000,
    .size = 0x1000,
    .callback = serial_uart_callback,
    .init = serial_uart_init,
});
