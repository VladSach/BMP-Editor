#ifndef APP_H
#define APP_H

#include <string>
#include <SDL.h>
#include "SDL_opengl.h"

#include "defines.h"
#include "Gui.h"

class App {
private:
    const u32 screen_width;
    const u32 screen_height;

    SDL_Window *window;
    SDL_Surface *screen_surface = NULL; //The surface contained by the window
    SDL_Surface *image_surface = NULL; //The image we will load and show on the screen
    
    GLuint gl_texture = 0;

    Gui *gui;
public:

    App(u32 app_width, u32 app_height)
        : screen_width(app_width), screen_height(app_height) {}
    ~App() = default;

    bool init(const char *title, Uint32 flags);
    void update();
    void render();
    void shutdown();

    bool loadBMP(const char *path);
    bool handleInput();
};

#endif // APP_H
