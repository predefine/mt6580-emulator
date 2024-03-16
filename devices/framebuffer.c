#include <log.h>
#include <devices.h>
#include <unicorn/unicorn.h>
#include <SDL2/SDL.h>

#define WIDTH 800
#define HEIGHT 1280
#define FPS 60

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* fbtext;
uint8_t update;

int framebuffer_update(void* arg);

SDL_Thread* update_thread;

void framebuffer_init(uc_engine* uc, void* devptr){
    (void)uc;
    (void)devptr;

    if(SDL_VideoInit(NULL) != 0)
        PANIC_MSG("SDL2 Video Subsystem init failed!\n");

    window = SDL_CreateWindow("MT6580 Emulator",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if(window == NULL)
        PANIC_MSG("SDL2 window init failed!\n");
    renderer = SDL_CreateRenderer(window, -1,  SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
    if(renderer == NULL)
        PANIC_MSG("SDL2 renderer init failed!\n");
    fbtext = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
    SDL_RenderClear(renderer);
    update = 1;
    update_thread = SDL_CreateThread(framebuffer_update, "framebuffer_update", NULL);
    SDL_DetachThread(update_thread);
}

int framebuffer_update(void* arg){
    (void)arg;
    while(update){
        SDL_RenderCopy(renderer, fbtext, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(1000/FPS);
    }
    return 0;
}

void framebuffer_callback(uc_engine* uc, uc_mem_type type, uint64_t address, int size, long valuel, void* user_data){
    (void)uc;
    if(type != UC_MEM_WRITE) return;
    (void)size;
    (void)user_data;
    uint32_t value = valuel;
    device* dev = user_data;
    uint32_t screen_pos = address - dev->address;
    void* pixels;
    int pitch;
    SDL_LockTexture(fbtext, NULL, &pixels, &pitch);
    if(size == 4){
        ((uint32_t*)pixels)[screen_pos >> 2] = value;
    } else if (size == 1){
        ((char*)pixels)[screen_pos] = value;
    } else {
        PANIC_MSG("not supported size %d!\n", size);
    }
    SDL_UnlockTexture(fbtext);
    return;
}

void framebuffer_exit(uc_engine* uc, void* devptr){
    (void)uc;
    (void)devptr;
    update = 0;
    SDL_WaitThread(update_thread, NULL);
}

device devices_framebuffer = {
    .address = FRAMEBUFFER_ADDRESS,
    .size = WIDTH*HEIGHT*4,
    .init = framebuffer_init,
    .callback = framebuffer_callback,
    .exit = framebuffer_exit,
    .name = "Display.Framebuffer"
};
