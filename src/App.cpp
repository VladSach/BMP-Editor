#include "App.h"
#include "SDL_opengl.h"
#include <stdio.h>

bool App::init(const char *title, Uint32 window_flags)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    window = SDL_CreateWindow(title,
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              screen_width, screen_height,
                              window_flags);

    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    screen_surface = SDL_GetWindowSurface(window);
    gui = new Gui(window);
    gui->init();

    return true;
}

void App::update()
{
    gui->update();
    //SDL_BlitSurface(image_surface, NULL, screen_surface, NULL);
}

void App::render()
{
    glGenTextures(1, &gl_texture);
    glBindTexture(GL_TEXTURE_2D, gl_texture);
    
    int gl_texture_mode = GL_RGB;
    if (image_surface->format->BytesPerPixel == 4) {
        gl_texture_mode = GL_RGBA;
    }
     
    glTexImage2D(GL_TEXTURE_2D, 0, gl_texture_mode,
                image_surface->w, image_surface->h, 0,
                gl_texture_mode, GL_UNSIGNED_BYTE,
                image_surface->pixels);
     
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gl_texture);

    // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT);
    
    // int win_width, win_height;
    // SDL_GetWindowSize(window, &win_width, &win_height);
    // glViewport(0.0, 0.0, win_width * 0.75, win_height);

    // Mapping texture
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f, 1.0f); // top left
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f, 1.0f); // top right
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1.0f, 1.0f); // bottom right
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, 1.0f); // bootom left
    glEnd();

    gui->render();
    SDL_GL_SwapWindow(window);
}

void App::shutdown()
{
    glDeleteTextures(1, &gl_texture);

    gui->cleanup();

    SDL_FreeSurface(image_surface);
    image_surface = NULL;

    SDL_DestroyWindow(window);
    window = NULL;

    SDL_Quit();
}

bool App::loadBMP(const char *path)
{
    SDL_Surface *temp = SDL_LoadBMP(path);

    if (!temp) {
        printf("Unable to load image %s! SDL_Error: %s\n", path, SDL_GetError());
        return false;
    }

    // Changing the surface to RGB format
    Uint32 rmask, gmask, bmask, amask;
    /* SDL interprets each pixel as a 32-bit number,
     * so our masks must depend on the endianness (byte order) 
     * of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    image_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 
                                         temp->w, temp->h, 32,
                                         rmask, gmask, bmask, amask);

    SDL_BlitSurface(temp, NULL, image_surface, NULL);
    SDL_FreeSurface(temp);
    // Now surface is in RGB format!

    return true;
}

bool App::handleInput()
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL2_ProcessEvent(&e);

        switch (e.type) {
        case SDL_QUIT:
            return true;
            break;

        default:
            break;
        }
    }

    return false;
}
