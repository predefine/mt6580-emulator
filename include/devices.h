#pragma once

#include <stddef.h>
#include <stdint.h>
#include <unicorn/unicorn.h>

#define FRAMEBUFFER_ADDRESS 0xbf100000

typedef struct {
    uint64_t address;
    uint64_t size;
    uc_cb_hookmem_t callback;
    char name[128];
    void (*init)(uc_engine* uc, void* dev);
    void (*exit)(uc_engine* uc, void* dev);
} device;

typedef struct _devices_list{
    device* this;
    struct _devices_list* next;
    size_t next_count;
    size_t total;
} devices_list;

const devices_list* get_devices();
void add_device(char* name, device dev);

#define DEVICES_INIT_PRIOR 101
#define DEVICES_APPEND_PRIOR 102

#define DEVICE(name, ...) \
    static void __attribute__((constructor(DEVICES_APPEND_PRIOR))) devices_init_dev_##name (void) { \
        add_device(#name, (device)__VA_ARGS__); \
    }
