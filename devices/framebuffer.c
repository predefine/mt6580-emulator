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
    device* dev = devptr;
    window = SDL_CreateWindow("MT6580 Emulator",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if(window == NULL){
        printf("SDL2 window init failed!\n");
        exit(-1);
    }
    renderer = SDL_CreateRenderer(window, -1,  SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
    if(renderer == NULL){
        printf("SDL2 renderer init failed!\n");
        exit(-1);
    }
    fbtext = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
    SDL_RenderClear(renderer);
    update = 1;
    update_thread = SDL_CreateThread(framebuffer_update, "framebuffer_update", NULL);
    SDL_DetachThread(update_thread);
}

int framebuffer_update(void* arg){
    while(update){
        SDL_RenderCopy(renderer, fbtext, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(1000/FPS);
    }
    return 0;
}

void framebuffer_callback(uc_engine* uc, uc_mem_type type, uint64_t address, int size, long valuel, void* user_data){
    uint32_t value = valuel;
    device* dev = user_data;
    uint32_t screen_pos = address - dev->address;
    void* pixels;
    int pitch;
    SDL_LockTexture(fbtext, NULL, &pixels, &pitch);
    *((uint32_t*)&pixels[screen_pos]) = value;
    SDL_UnlockTexture(fbtext);
    return;
}

void framebuffer_exit(uc_engine* uc, void* devptr){
    update = 0;
    SDL_WaitThread(update_thread, NULL);
}

device devices_framebuffer = {
    .address = 0xbf100000,
    .size = WIDTH*HEIGHT*4,
    .init = framebuffer_init,
    .callback = framebuffer_callback,
    .exit = framebuffer_exit,
    .name = "Display.Framebuffer"
};
