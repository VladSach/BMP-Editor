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

    if (!app.init("BMP Viewer", SDL_WINDOW_SHOWN)) {
        printf("Failed to initialize!\n");
        return 1;
    }
    
    app.run();

    app.destroy();

    return 0;
}
