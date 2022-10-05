#ifndef APP_H
#define APP_H

#include <string>
#include <SDL.h>
#include "SDL_opengl.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"

#include "defines.h"

class App {
private:
    const u32 screen_width;
    const u32 screen_height;

    SDL_Window *window;
    SDL_Surface *screen_surface = NULL; //The surface contained by the window
    SDL_Surface *image_surface = NULL; //The image we will load and show on the screen
    
    ImGuiIO* io;
    SDL_GLContext gl_context;

    GLuint gl_texture = 0;

public:
    App(u32 app_width, u32 app_height)
        : screen_width(app_width), screen_height(app_height) {}
    ~App() = default;

    bool init(const char *title, Uint32 flags);
    void initImGui();

    void run();

    void update();
    void render();

    void destroy();

    bool loadBMP(const char *path);
    bool handleInput();
};

#endif // APP_H
