#include <devices.h>
#include <unicorn/unicorn.h>

#define TIMER_CLR 1<<1
#define TIMER_EN 1<<0

#define TIMER_FLAGS_DISABLED 1<<0
#define TIMER_FLAGS_CLOCK_SOURCE 1<<1

typedef enum {
    GPT_MODE_KEEPGO,
    GPT_MODE_ONESHOT,
    GPT_MODE_FREERUN,
    GPT_MODE_REPEAT
} gpt_mode;

typedef struct {
    uint32_t timer_ticks;
    uint32_t timer_compare;
    char timer_flags;
    gpt_mode mode;
} gpt_timer;

typedef enum {
    GPT_CONTROL,
    GPT_CLK,
    GPT_COUNTER,
    GPT_COMPARE
}gpt_timer_action;

char clock_divs[0xf] = {1,2,3,4,5,6,8,9,10,11,12,13,16,32,64};

gpt_timer timers[6];

void timer_tick(){
    for(int i = 0; i < 6; i++)
        if (!(timers[i].timer_flags & TIMER_FLAGS_DISABLED)){
            timers[i].timer_ticks += 2000;//clock_divs[(timers[i].timer_flags>>2)&0xf];
            int call_int = timers[i].mode != GPT_MODE_FREERUN;//????
            if(timers[i].timer_compare >= timers[i].timer_ticks)
                switch (timers[i].mode){
                    case GPT_MODE_ONESHOT:
                        timers[i].timer_flags |= TIMER_FLAGS_DISABLED;
                        break;
                    case GPT_MODE_REPEAT:
                        timers[i].timer_ticks = 0;
                        break;
                    default:
                        break;
                }
            (void)call_int; //?????????
        }
}

#define register_to_gpt_number(reg) ((int)reg)>>4
#define register_to_gpt_action(reg) ((gpt_timer_action)(((int)reg) & 0xf)>>2)

void timers_gpt_callback (uc_engine* uc, uc_mem_type type, uint64_t address, int size, int64_t valuel, void* user_data){
    timer_tick();
    device* dev = (device*) user_data;
    (void)dev;
    (void)uc;
    uint64_t reg = address - dev->address;
    (void)size;
    (void)reg;
    uint32_t value = valuel;
    int gpt = register_to_gpt_number(reg);
    gpt = (gpt==7?6:gpt)-1; // bcs gpt6 is 64 bit
    if (gpt < 0 || gpt > 5)
        return;
    gpt_timer_action action = register_to_gpt_action(reg);
    if(type == UC_MEM_WRITE){
        switch (action){
            case GPT_CONTROL:
                if(value & TIMER_EN)
                    timers[gpt].timer_flags &= ~(TIMER_FLAGS_DISABLED);
                else
                    timers[gpt].timer_flags |= TIMER_FLAGS_DISABLED;
                break;
            case GPT_CLK:
                timers[gpt].timer_flags &= TIMER_FLAGS_CLOCK_SOURCE;
                if(value>>4)
                    timers[gpt].timer_flags |= TIMER_FLAGS_CLOCK_SOURCE;
                timers[gpt].timer_flags |= (value&0xf) << 2;
                break;
            case GPT_COMPARE:
                timers[gpt].timer_compare = value;
                break;
            default:
                break;
        }
    }
    if (type == UC_MEM_READ){
        uint32_t temp;
        switch (action){
            case GPT_CONTROL:
                temp = timers[gpt].timer_flags & ~(TIMER_FLAGS_DISABLED);
                uc_mem_write(uc, address, &temp, sizeof(uint32_t));
                break;
            case GPT_COUNTER:
                temp = timers[gpt].timer_ticks;
                uc_mem_write(uc, address, &temp, sizeof(uint32_t));
                break;
            default:
                break;
        }
    }
}

void timers_gpt_init(uc_engine* uc, void* devptr){
    (void)uc;
    (void)devptr;
    for(int i = 0; i < 6; i++)
        timers[i].timer_flags |= TIMER_FLAGS_DISABLED;
}


DEVICE(TIMERS_GPT, {
    .address = 0x10008000,
    .size = 0x400,
    .callback = timers_gpt_callback,
    .init = timers_gpt_init
});
