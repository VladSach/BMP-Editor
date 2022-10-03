#include "SDL_events.h"
#include "SDL_stdinc.h"
#include "SDL_timer.h"
#include <SDL.h>
#include <stdio.h>
#include <string>

#include "defines.h"
#include "App.h"

int main(int argc, char* args[])
{
    App app(1280, 720);
    const char img1_path[] = "image/img1.bmp";

    if (!app.init("BMP Viewer", SDL_WINDOW_SHOWN)) {
        printf("Failed to initialize!\n");
        return 1;
    }

    if (!app.loadBMP(img1_path)) {
        printf("Failed to load image!\n");
        return 1;
    }

    bool quit = false;

    // const u64 fps = 120 / 1000; // FPS in ms
    // Uint64 elapsed = 0;
    // Uint64 frame_time = 0;

    while (!quit) {
        //elapsed = SDL_GetTicks64();
    
        quit = app.handleInput();
    
        app.update();
        app.render();

        // frame_time = SDL_GetTicks64() - elapsed;
        // if (fps > frame_time) {
        //     SDL_Delay(fps - frame_time);
        // }
    }

    app.shutdown();

    return 0;
}
