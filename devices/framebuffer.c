#include <log.h>
#include <devices.h>
#include <signal.h>
#include <unicorn/unicorn.h>
#include <pthread.h>
#include <SFML/Graphics.h>

#define WIDTH 800
#define HEIGHT 1280
#define FPS 60

sfRenderWindow* window;
sfImage* fbImage;
sfTexture* fbTexture;
sfSprite* fbSprite;
pthread_t render_thread;
pthread_attr_t render_thread_attr;

void* framebuffer_update(void* arg);

void framebuffer_init(uc_engine* uc, void* devptr){
    (void)uc;
    (void)devptr;
    window = sfRenderWindow_create(
        (sfVideoMode){.width = WIDTH, .height = HEIGHT, .bitsPerPixel = 32},
        "MT6580 Emulator", 0, NULL
    );
    if(!window)
        PANIC_MSG("CSFML window init failed!\n");
    fbImage = sfImage_create(WIDTH, HEIGHT);
    if(!fbImage)
        PANIC_MSG("CSFML image create failed!\n");
    fbTexture = sfTexture_create(WIDTH, HEIGHT);
    if(!fbTexture)
        PANIC_MSG("CSFML texture create failed!\n");
    fbSprite = sfSprite_create();
    if(!fbSprite)
        PANIC_MSG("CSFML sprite create failed!\n");
    pthread_attr_init(&render_thread_attr);
    pthread_create(&render_thread, &render_thread_attr, framebuffer_update, NULL);
    pthread_detach(render_thread);
}

void* framebuffer_update(void* arg){
    (void)arg;
    sfEvent event;
    while(sfRenderWindow_isOpen(window)){
        while(sfRenderWindow_pollEvent(window, &event))
            if(event.type == sfEvtClosed)
                raise(SIGINT);
        sfRenderWindow_clear(window, (sfColor){});
        sfTexture_updateFromImage(fbTexture, fbImage, 0, 0);
        sfSprite_setTexture(fbSprite, fbTexture, 0);
        sfRenderWindow_drawSprite(window, fbSprite, NULL);
        sfRenderWindow_display(window);
        sfSleep((sfTime){.microseconds = 1000000 / FPS});
    }
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
    const uint8_t* pixels = sfImage_getPixelsPtr(fbImage);
    if(size == 4){
        ((uint32_t*)pixels)[screen_pos >> 2] = value;
    } else if (size == 1){
        ((uint8_t*)pixels)[screen_pos] = value;
    } else {
        PANIC_MSG("not supported size %d!\n", size);
    }
    return;
}

void framebuffer_exit(uc_engine* uc, void* devptr){
    (void)uc;
    (void)devptr;
    sfRenderWindow_destroy(window);
}

device devices_framebuffer = {
    .address = FRAMEBUFFER_ADDRESS,
    .size = WIDTH*HEIGHT*4,
    .init = framebuffer_init,
    .callback = framebuffer_callback,
    .exit = framebuffer_exit,
    .name = "Display.Framebuffer"
};
