#include <unicorn/unicorn.h>
#include <unicorn/arm.h>
#include <log.h>
// #include <udbserver.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <devices.h>

#define BOOTVECTOR_RESET KERNEL_OFFSET


#define UBOOT_OFFSET (0x81e00000-0x200)
#define UBOOT_SIZE 0x800000
#define UBOOT_FILE "../u-boot/out/u-boot-mtk.bin"

#define KERNEL_OFFSET 0x80008000
#define KERNEL_DTB_OFFSET (KERNEL_OFFSET + 0x8000000)
#define KERNEL_SIZE 0x800000
#define KERNEL_FILE "kernel"

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
    uint32_t r_pc;
    uc_reg_read(uc, UC_ARM_REG_PC, &r_pc);
    printf("INFO! detected %s at 0x%lx-0x%lx with value=0x%lx at pc=0x%x\n", type == UC_MEM_READ ? "read" : "write", address, address+size, value, r_pc);
}
#endif

#define EXTRACT_BYTE(x, n)	((unsigned long long)((uint8_t *)&x)[n])
#define CPU_TO_FDT32(x) ((EXTRACT_BYTE(x, 0) << 24) | (EXTRACT_BYTE(x, 1) << 16) | \
			 (EXTRACT_BYTE(x, 2) << 8) | EXTRACT_BYTE(x, 3))

uc_engine* engine;

void emu_exit(){
    printf("Emulator terminating...\n");
    for(int i = 0; devices[i] != NULL; i++){
        device* dev = devices[i];
        if(dev->exit)
            dev->exit(engine, (void*)dev);
    }
    exit(0);
}

void emu_start(uint32_t start){
    printf("Starting emulator...\n");
    signal(SIGINT, emu_exit);
    uc_err err = uc_emu_start(engine, start, 0xffffffff, 0, 0);
    if (err) {
        printf("Failed on uc_emu_start() with error returned %u: %s\n",
        err, uc_strerror(err));
    }

    uint32_t r_pc;
    uc_reg_read(engine, UC_ARM_REG_PC, &r_pc);
    printf("PC: 0x%x\n", r_pc);
}

void emu_init(){
    if(uc_open(UC_ARCH_ARM, UC_MODE_LITTLE_ENDIAN | UC_MODE_THUMB, &engine) != UC_ERR_OK){
        printf("uc_open failed :(\n");
        exit(-1);
    }
    if(uc_ctl_set_cpu_model(engine, UC_CPU_ARM_CORTEX_A7) != UC_ERR_OK){
        printf("uc_ctl_set_cpu_model failed :(\n");
        exit(-1);
    }
}

void map_mem(uint64_t address, uint64_t size, char ignore_err){
    uc_err err = uc_mem_map(engine, address, size, UC_PROT_ALL);
    if (err && !ignore_err)
        PANIC_MSG("Failed map memory at 0x%lx, size 0x%lx with error returned %u: %s\n", address, size, err, uc_strerror(err));
}

void load_file(char* filename, uint64_t address, uint64_t size){
    char *data = (char*)malloc(size);
    int fd;
    if ((fd = open(filename, O_RDONLY)) < 0)
        PANIC_MSG("load_file: open file failed :(\n");
    if (read(fd, data, size) < 1)
        PANIC_MSG("load_file: read failed :(\n");
    close(fd);
    uc_err err = uc_mem_write(engine, address, data, size);
    if (err)
        PANIC_MSG("load_file: Failed write memory with error returned %u: %s\n", err, uc_strerror(err));
    DEBUG_MSG("load_file: memory writed!\n");
    free(data);
}

void map_mem_and_load_file(char* filename, uint64_t address, uint64_t size){
    map_mem(address, size, 1);
    load_file(filename, address, size);
}

void devices_probe(){
    for(int i = 0; devices[i] != NULL; i++){
        device* dev = devices[i];
        printf("adding device %d: %s...\n", i, dev->name);
        map_mem(dev->address, dev->size, 1);
        if(dev->callback){
            printf("Adding device %d callback...\n", i);
            uc_hook* hook = malloc(sizeof(uc_hook));
            uc_err err = uc_hook_add(engine, hook, UC_HOOK_MEM_READ | UC_HOOK_MEM_WRITE, dev->callback, dev, dev->address,
                    dev->address + dev->size - 1);
            if (err){
                printf("Adding device %d: adding hook failed with error %u!\n", i, err);
            }
        }
        if(dev->init)
            dev->init(engine, (void*)dev);
    }
}

void add_hooks(){
    uc_hook mem_unmapped_read_hook;
    uc_err err = uc_hook_add(engine, &mem_unmapped_read_hook,
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
}

void find_dtb_in_kernel(uint32_t offset, uint32_t* dtb_offset, uint32_t* dtb_size){
    uint32_t kernel_load_address, kernel_end_address;
    uc_mem_read(engine, offset + 0x28, &kernel_load_address, sizeof(kernel_load_address));
    uc_mem_read(engine, offset + 0x2c, &kernel_end_address, sizeof(kernel_end_address));
    uint32_t kernel_size = kernel_end_address - kernel_load_address;
    *dtb_offset = offset + kernel_size;
    uint32_t _dtb_size;
    uc_mem_read(engine, (*dtb_offset)+0x4, &_dtb_size, sizeof(_dtb_size));
    *dtb_size = CPU_TO_FDT32(_dtb_size);
}

void copy_mem(uint32_t offset, uint32_t new_offset, uint32_t size){
    uint32_t buffer;
    for(uint32_t i = 0; i < size; i += sizeof(buffer)){
        uc_mem_read(engine, offset + i, &buffer, sizeof(buffer));
        uc_mem_write(engine, new_offset + i, &buffer, sizeof(buffer));
    }
}

int main(){
    emu_init();

    add_hooks();
    // 64 kb SRAM
    map_mem(SRAM_OFFSET, SRAM_SIZE, 0);
    // RAM
    map_mem(RAM_OFFSET, RAM_SIZE, 0);

    load_file(UBOOT_FILE, UBOOT_OFFSET, UBOOT_SIZE);
    load_file(KERNEL_FILE, KERNEL_OFFSET, KERNEL_SIZE);

    devices_probe();

    uint32_t dtb_offset, dtb_size, remap_offset;
    remap_offset = KERNEL_DTB_OFFSET;
    find_dtb_in_kernel(KERNEL_OFFSET, &dtb_offset, &dtb_size);
    copy_mem(dtb_offset, remap_offset, dtb_size);
    uc_reg_write(engine, UC_ARM_REG_R2, &remap_offset);

    emu_start(BOOTVECTOR_RESET);

    return 0;
}
