#pragma once

#include <stddef.h>
#include <stdint.h>
#include <unicorn/unicorn.h>

typedef struct {
    uint64_t address;
    uint64_t size;
    uc_cb_hookmem_t callback;
    char name[128];
    void (*init)(uc_engine* uc, void* dev);
    void (*exit)(uc_engine* uc, void* dev);
} device;

extern device *devices[];
