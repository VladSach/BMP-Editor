#ifndef GUI_H
#define GUI_H

#include <SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"

class Gui {
private:
    SDL_Window* const window;
    ImGuiIO* io;
    SDL_GLContext gl_context;

public:
    Gui(SDL_Window* const sdl_window)
        : window(sdl_window) {}
    ~Gui() = default;

    void init();
    void update();
    void render();
    void cleanup();
};

#endif // GUI_H
