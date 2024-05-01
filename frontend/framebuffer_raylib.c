#include <pthread.h>
#include <log.h>
#include <devices.h>
#include <signal.h>
#include <unicorn/unicorn.h>
#include <raylib.h>

#define WIDTH 800
#define HEIGHT 1280
#define FPS 60

uint8_t update;
unsigned char* fb_buf;
Texture2D fb_texture;
pthread_t render_thread;

void* framebuffer_update(void* arg);

void framebuffer_init(uc_engine* uc, void* devptr){
    (void)uc;
    (void)devptr;
    fb_buf = malloc(WIDTH * HEIGHT * 4);
    pthread_attr_t render_attr;
    update = 1;
    pthread_attr_init(&render_attr);
    pthread_create(&render_thread, &render_attr, framebuffer_update, NULL);
    pthread_detach(render_thread);
}

void* framebuffer_update(void* arg){
    (void)arg;
    InitWindow(WIDTH, HEIGHT, "MT6580 Emulator");
    SetTargetFPS(FPS);
    Image fb_img = {
        .data = fb_buf,
        .width = WIDTH,
        .height = HEIGHT,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    fb_texture = LoadTextureFromImage(fb_img);
    while(update && !WindowShouldClose()){
        UpdateTexture(fb_texture, fb_buf);
        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexture(fb_texture, 0, 0, WHITE);
        EndDrawing();
        WaitTime(((double)1) / FPS);
    }
    UnloadTexture(fb_texture);
    free(fb_buf);
    if(update)
        raise(SIGINT);
    return NULL;
}

void framebuffer_callback(uc_engine* uc, uc_mem_type type, uint64_t address, int size, long valuel, void* user_data){
    (void)uc;
    if(type != UC_MEM_WRITE) return;
    (void)size;
    (void)user_data;
    uint32_t value = valuel;
    device* dev = user_data;
    uint32_t screen_pos = address - dev->address;
    if(size == 4){
        ((uint32_t*)fb_buf)[screen_pos >> 2] = value;
    } else if (size == 1){
        ((uint8_t*)fb_buf)[screen_pos] = value;
    } else {
        PANIC_MSG("not supported size %d!\n", size);
    }
    return;
}

void framebuffer_exit(uc_engine* uc, void* devptr){
    (void)uc;
    (void)devptr;
    update = 0;
    CloseWindow();
}

DEVICE(DISPLAY_FRAMEBUFFER, {
    .address = FRAMEBUFFER_ADDRESS,
    .size = WIDTH*HEIGHT*4,
    .init = framebuffer_init,
    .callback = framebuffer_callback,
    .exit = framebuffer_exit,
});
