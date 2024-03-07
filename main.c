#include <unicorn/unicorn.h>
#include <unicorn/arm.h>
// #include <udbserver.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <devices.h>

#define BOOTVECTOR_RESET 0x81e00000

// #define BOOTROM_OFFSET 0
// #define BOOTROM_SIZE 0x100000
// #define BOOTROM_FILE "brom_6580.bin"

// #define PRELOADER_OFFSET (0x201000)
// #define PRELOADER_SIZE (128*1024)
// #define PRELOADER_FILE "/mnt/alps/alps/out/target/product/pmt3151/obj/PRELOADER_OBJ/bin/preloader_pmt3151_LINKED.bin"

#define UBOOT_OFFSET 0x81e00000-0x200
#define UBOOT_SIZE 0x800000
#define UBOOT_FILE "../u-boot/out/u-boot-mtk.bin"
// #define UBOOT_FILE "lk.bin"

#define RAM_OFFSET 0x80000000
#define RAM_SIZE 0x80000000

#define SRAM_OFFSET 0x0100000
#define SRAM_SIZE 64*1024

void mem_read_unmapped(uc_engine* uc, uc_mem_type type, uint64_t address, int size, long value, void* user_data){
    (void)uc;
    (void)size;
    (void)value;
    (void)user_data;
    printf("ERROR! detected %s on unmapped 0x%lx-0x%lx\n", type == UC_MEM_READ_UNMAPPED ? "read" : "write", address, address + size);
}

#ifdef DEBUG_MEM
void mem_info(uc_engine* uc, uc_mem_type type, uint64_t address, int size, long value, void* user_data){
    (void)uc;
    (void)size;
    (void)value;
    (void)user_data;
    (void)type;
    if(type == UC_MEM_READ)
        uc_mem_read(uc, address, &value, size);
    //if(address < RAM_OFFSET || address > RAM_OFFSET + RAM_SIZE - 1)){
    if((address < 0x11005000 || address > 0x11007000) && (address < RAM_OFFSET ) && address > 64*1024+SRAM_OFFSET){
        uint32_t r_pc;
        uc_reg_read(uc, UC_ARM_REG_PC, &r_pc);
        printf("INFO! detected %s at 0x%lx-0x%lx with value=0x%lx at pc=0x%x\n", type == UC_MEM_READ ? "read" : "write", address, address+size, value, r_pc);
    }
}
#endif

int main(){
    uc_engine* engine;
    uc_err err;

    if(uc_open(UC_ARCH_ARM, UC_MODE_LITTLE_ENDIAN | UC_MODE_THUMB, &engine) != UC_ERR_OK){
        printf("uc_open failed :(");
        return -1;
    }
    if(uc_ctl_set_cpu_model(engine, UC_CPU_ARM_CORTEX_A7) != UC_ERR_OK){
        printf("uc_ctl_set_cpu_model failed :(");
        return -1;
    }

    // 64 kb sram
    err = uc_mem_map(engine, SRAM_OFFSET, SRAM_SIZE, UC_PROT_ALL);
    if (err) {
        printf("Failed map on-chip sram with error returned %u: %s\n", err, uc_strerror(err));
        return -1;
    }

    // 128 mb ram
    err = uc_mem_map(engine, RAM_OFFSET, RAM_SIZE, UC_PROT_ALL);
    if (err) {
        printf("Failed map ram with error returned %u: %s\n", err, uc_strerror(err));
        return -1;
    }
    printf("ram mapped!\n");


    int fd;

    //Bootrom
    // char *bootrom = (char*)malloc(BOOTROM_SIZE*sizeof(char));
    // int fd;
    // if ((fd = open(BOOTROM_FILE, O_RDONLY)) < 0){
    //     printf("bootrom open file failed :(");
    //     return -1;
    // }
    // int bootrom_size = 0;
    // if ((bootrom_size = read(fd, bootrom, BOOTROM_SIZE)) < 1){
    //     printf("bootrom read failed :(");
    //     return -1;
    // }
    // printf("brom size: 0x%x\n", bootrom_size);
    // close(fd);
    // err = uc_mem_map(engine, BOOTROM_OFFSET, bootrom_size, UC_PROT_ALL);
    // if (err) {
    //     printf("Failed map bootrom with error returned %u: %s\n", err, uc_strerror(err));
    //     return -1;
    // }
    // printf("bootrom memory mapped!\n");
    // err = uc_mem_write(engine, BOOTROM_OFFSET, bootrom, bootrom_size);
    // if (err) {
    //     printf("Failed write bootrom with error returned %u: %s\n", err, uc_strerror(err));
    //     return -1;
    // }
    // printf("bootrom memory writed!\n");
    // free(bootrom);

    // Preloader
    // char *preloader = (char*)malloc(PRELOADER_SIZE*sizeof(char));
    // if ((fd = open(PRELOADER_FILE, O_RDONLY)) < 0){
    //     printf("preloader open file failed :(");
    //     return -1;
    // }
    // if (read(fd, preloader, PRELOADER_SIZE) < 1){
    //     printf("preloader read failed :(");
    //     return -1;
    // }
    // close(fd);
    // err = uc_mem_map(engine, PRELOADER_OFFSET, PRELOADER_SIZE, UC_PROT_ALL);
    // if (err) {
    //     printf("Failed map preloader with error returned %u: %s\n", err, uc_strerror(err));
    //     return -1;
    // }
    // printf("preloader memory mapped!\n");
    // err = uc_mem_write(engine, PRELOADER_OFFSET, preloader, PRELOADER_SIZE);
    // if (err) {
    //     printf("Failed write preloader with error returned %u: %s\n", err, uc_strerror(err));
    //     return -1;
    // }
    // printf("preloader memory writed!\n");
    // free(preloader);

    // Uboot
    char *uboot = (char*)malloc(UBOOT_SIZE*sizeof(char));
    if ((fd = open(UBOOT_FILE, O_RDONLY)) < 0){
        printf("uboot open file failed :(");
        return -1;
    }
    if (read(fd, uboot, UBOOT_SIZE) < 1){
        printf("uboot read failed :(");
        return -1;
    }
    close(fd);
    err = uc_mem_write(engine, UBOOT_OFFSET, uboot, UBOOT_SIZE);
    if (err) {
        printf("Failed write uboot with error returned %u: %s\n", err, uc_strerror(err));
        return -1;
    }
    printf("uboot memory writed!\n");
    free(uboot);



    uc_hook mem_unmapped_read_hook;
    err = uc_hook_add(engine, &mem_unmapped_read_hook,
                UC_HOOK_MEM_UNMAPPED, mem_read_unmapped, NULL, 1, 0);
    if (err){
         printf("unmapped read&write hook add failed with error %u %s!\n", err, uc_strerror(err));
    }

#ifdef DEBUG_MEM
    uc_hook mem_info_hook;
    err = uc_hook_add(engine, &mem_info_hook,
                UC_HOOK_MEM_READ | UC_HOOK_MEM_WRITE, mem_info, NULL, 1, 0);
    if (err){
         printf("mem read&write hook add failed with error %u %s!\n", err, uc_strerror(err));
    }
#endif

    printf("adding devices...\n");
    for(int i = 0; devices[i] != NULL; i++){
        device* dev = devices[i];
        printf("adding device %d: %s...\n", i, dev->name);
        err = uc_mem_map(engine, dev->address, dev->size, UC_PROT_READ | UC_PROT_WRITE);
        if(err){
            printf("Adding device %d: mapping memory failed with error %u: %s!\n", i, err, uc_strerror(err));
            continue;
        }
        if(dev->callback){
            printf("Adding device %d callback...\n", i);
            // if(dev->hook == NULL)
            uc_hook* hook = malloc(sizeof(uc_hook));
            err = uc_hook_add(engine, hook, UC_HOOK_MEM_READ | UC_HOOK_MEM_WRITE, dev->callback, dev, dev->address,
                    dev->address + dev->size - 1);
            if (err){
                printf("Adding device %d: adding hook failed with error %u!\n", i, err/*uc_strerror(err)*/);
                continue;
            }
        }
        if(dev->init)
            dev->init(engine, (void*)dev);
    }
    printf("Devices added!\n");

    // printf("adding udbserver hook... ");
    // udbserver(engine, 1235, BOOTVECTOR_RESET);
    // printf("Done!\n");

    printf("Starting emulator...\n");

    err=uc_emu_start(engine, BOOTVECTOR_RESET, 0xffffffff, 0, 0);
    if (err) {
        printf("Failed on uc_emu_start() with error returned %u: %s\n",
        err, uc_strerror(err));
    }

    uint32_t r_pc;
    uc_reg_read(engine, UC_ARM_REG_PC, &r_pc);
    printf("PC: 0x%x\n", r_pc);
    // printf("Total ticks: %d\n", timer_gpt_ticks);

    return 0;
}
