#include <log.h>
#include <devices.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define OCR_BUSY 0x80000000
#define OCR_HCS 0x40000000

#define SD_BLOCKS 262144
#define SD_BLOCK_SHIFT 9
#define SD_BLOCKSIZE (1<<SD_BLOCK_SHIFT)

#define min(a,b) ((a)<(b)?(a):(b))

typedef struct {
    char app_cmd;
    char cmd;
    uint32_t arg;
    uint32_t resp[4];
    uint32_t int_flags;
    uint32_t blocks;
} mmc_cmd;

typedef struct {
    void* buffer_ptr;
    // max buffer size is 256 kilobytes
    uint32_t buffer_used;
} mmc_data_buffer;

typedef struct {
    mmc_cmd cmd;
    mmc_data_buffer data_buff;

    int mmc_fd;
    uint32_t mmc_size;
} msdc_device;

static msdc_device msdc_devices[2];

void msdc_push_data(msdc_device* device, void* data, size_t count){
    if((size_t)device->data_buff.buffer_used + count > ((1<<16)-1))
        return;
    memcpy(device->data_buff.buffer_ptr+device->data_buff.buffer_used, data, count);
    device->data_buff.buffer_used += count;
}

void mmc_handle_acmd(msdc_device* device){
    uint32_t data;
    switch(device->cmd.cmd){
        case 6:
            break;
        case 13:
            #warning "ACMD 13 is not implemented correctly!"
            data = 0;
            for(int i = 0; i < 16; i++)
                msdc_push_data(device, &data, sizeof(data));
            break;
        case 41:
            device->cmd.resp[0] = OCR_BUSY | OCR_HCS;
            break;
        case 51:
            data = htonl((2 << 24) | (1 << 15));
            msdc_push_data(device, &data, sizeof(data));
            data = 0;
            msdc_push_data(device, &data, sizeof(data));
            break;
        default:
            PANIC_MSG("mmc: unhandled ACMD%d with arg=0x%x\n", device->cmd.cmd, device->cmd.arg);
            break;
    }
}
void msdc_read_blocks(msdc_device* device, uint32_t count){
    if(device->mmc_fd == 0){
        PANIC_MSG("mmc: fd of mmc disk image is 0\n");
    }
    uint32_t address = device->cmd.arg * SD_BLOCKSIZE;
    lseek(device->mmc_fd, address, SEEK_SET);
    void* buffer = malloc(SD_BLOCKSIZE * count);
    uint32_t to_push = read(device->mmc_fd, buffer, SD_BLOCKSIZE * count);
    msdc_push_data(device, buffer, to_push);
    free(buffer);
}

// void msdc_read_bootdata(msdc_device* device){
//     uint32_t fd = open("preloader_k80_bsp.bin", O_RDONLY);
//     uint32_t to_push = read(fd, device->data_buff.buffer_ptr, 128*1024);
//     device->data_buff.buffer_used = to_push;
// }

void mmc_handle_cmd(msdc_device* device){
    if(device->cmd.app_cmd){
        device->cmd.app_cmd = 0;
        mmc_handle_acmd(device);
        return;
    }
    uint32_t data;
    switch(device->cmd.cmd){
        //currently useless
        case 0:
            // if(device->cmd.arg == 0xfffffffa)
            //     msdc_read_bootdata(device);
        case 2:
        case 3:
        case 7:
        case 12: // CMD12 - transfer complete, useless now
            break;
        case 6:
            data = 0;
            for(int i = 0; i < 16; i++)
                msdc_push_data(device, &data, sizeof(data));
            break;
        case 16:
            if(device->cmd.arg != SD_BLOCKSIZE)
                PANIC_MSG("new blocklen %d is net equal to %d blocks!\n", device->cmd.arg, SD_BLOCKSIZE);
            break;
        case 17:
            msdc_read_blocks(device, 1);
            break;
        case 18:
            msdc_read_blocks(device, device->cmd.blocks);
            break;
        case 8:
            device->cmd.resp[0] = 0xaa;
            break;
        case 9:
            #define EMMC_IN_BLOCKS (device->mmc_size>>(10 + SD_BLOCK_SHIFT))
            device->cmd.resp[2] = (SD_BLOCK_SHIFT << 16) | ((EMMC_IN_BLOCKS >> 16) & 0x3f);
            device->cmd.resp[1] = (EMMC_IN_BLOCKS &0xffff) << 16;
            break;
        case 55:
            device->cmd.app_cmd = 1;
            break;
        default:
            PANIC_MSG("mmc: unhandled CMD%d with arg=0x%x\n", device->cmd.cmd, device->cmd.arg);
    }

}

void devices_msdc_emmc_callback (uc_engine* uc, uc_mem_type type, uint64_t address, int size, long valuel, void* user_data){
    device* dev = (device*) user_data;
    uint64_t reg = ((address - dev->address)>>2)<<2;
    if(type == UC_MEM_READ){
        uc_mem_read(uc, address, &valuel, sizeof(valuel));
    }
    uint32_t value = valuel;
    msdc_device* device = &msdc_devices[(dev->address>>16)&1];
    switch(reg){
        case 0x34:
            device->cmd.cmd = value & ((1<<6)-1);
            device->cmd.resp[0] = 0;
            device->cmd.resp[1] = 0;
            device->cmd.resp[2] = 0;
            device->cmd.resp[3] = 0;
            mmc_handle_cmd(device);
            for(int i = 0; i < 4; i++){
                    uc_mem_write(uc, dev->address + 0x40 + (i<<2), &(device->cmd.resp[i]), sizeof(device->cmd.resp[i]));
            }
            break;
        case 0x38:
            device->cmd.arg = value;
            break;
        case 0x0:
            value &= ~(1<<2);
            value |= (1<<7);
            value |= (1<<6);
            value |= (1<<1);
            uc_mem_write(uc, address, &value, sizeof(value));
            break;
        case 0x50:
            device->cmd.blocks = value;
            break;
        case 0x14:
            value &= ~(1<<31);
            value = min(device->data_buff.buffer_used, 0x80);
            uc_mem_write(uc, address, &value, sizeof(value));
            break;
        case 0x1c:
            if(type == UC_MEM_READ){
                if(device->data_buff.buffer_used < (uint32_t)size)
                    device->data_buff.buffer_used = size;
                uc_mem_write(uc, address, device->data_buff.buffer_ptr, size);
                device->data_buff.buffer_used -= size;
                memmove(device->data_buff.buffer_ptr, device->data_buff.buffer_ptr+size, device->data_buff.buffer_used);
            }
            break;
        case 0xc:
            if(type == UC_MEM_WRITE && (value & (1<<31))){
                memset(device->data_buff.buffer_ptr, 0, device->data_buff.buffer_used);
                device->data_buff.buffer_used = 0;
            }
            if(type == UC_MEM_READ){
                value = 1 << 8;
                if(device->data_buff.buffer_used <= 128)
                    value |= 1 << 12;
                uc_mem_write(uc, address, &value, sizeof(value));
                device->cmd.int_flags = 0;
            }
            break;
        default:
            break;
    }
    // printf("[%s] Register 0x%lx is %s with value 0x%x\n", dev->name, reg, type == UC_MEM_READ ? "readed" : "writed", value);
}

void devices_msdc_emmc_init(uc_engine* uc, void* devptr){
    device* dev = devptr;
    uint32_t def_patch_bit2 = 0x14801801;
    uc_mem_write(uc, dev->address + 0xb8, &def_patch_bit2, sizeof(def_patch_bit2));
    for(int i = 0; i < 2; i++){
        msdc_devices[i].data_buff.buffer_ptr = malloc(256*1024);
        msdc_devices[i].data_buff.buffer_used = 0;
    }

    msdc_devices[0].mmc_fd = open("emmc_build/emmc.img", O_RDONLY);
    if(msdc_devices[0].mmc_fd > 0){
        struct stat st;
        fstat(msdc_devices[0].mmc_fd, &st);
        msdc_devices[0].mmc_size = st.st_size;
    } else
        msdc_devices[0].mmc_size = 0;
}

DEVICE(MSDC_EMMC, {
    .address = 0x11120000,
    .size = 0x1000,
    .callback = devices_msdc_emmc_callback,
    .init = devices_msdc_emmc_init
});

DEVICE(MSDC_SDCARD, {
    .address = 0x11130000,
    .size = 0x1000,
    .callback = devices_msdc_emmc_callback,
    .init = devices_msdc_emmc_init
});
