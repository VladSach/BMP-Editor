#include "App.h"
#include "SDL_opengl.h"
#include "imgui_impl_opengl3.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

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
    initImGui();

    return true;
}

void App::initImGui()
{
    const char* glsl_version = "#version 450";

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO(); (void)io;
    //io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, &gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void App::run()
{
    const char img1_path[] = "image/img1.bmp";
    bool quit = false;

    if (!loadBMP(img1_path)) {
        printf("Failed to load image!\n");
    }

    const u64 fps = 120 / 1000; // FPS in ms
    Uint64 elapsed = 0;
    Uint64 frame_time = 0;

    while (!quit) {
        elapsed = SDL_GetTicks64();
    
        quit = handleInput();
    
        update();
        render();

        frame_time = SDL_GetTicks64() - elapsed;
        if (fps > frame_time) {
            SDL_Delay(fps - frame_time);
        }
    }
}

void App::update()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    
    char image_path[255] = {0};
    float bar_height = 0.0;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open")) { /* Do stuff */ }
            if (ImGui::MenuItem("Save"))   { /* Do stuff */ }
            if (ImGui::MenuItem("Close"))  { /* Do stuff */ }
            ImGui::EndMenu();
        }

        if (ImGui::InputText("##path_textbox", image_path, sizeof(image_path))) {
            // change image
        }

        bar_height = ImGui::GetWindowHeight();
        ImGui::EndMainMenuBar();
    }

    ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoMove                |
                                 ImGuiWindowFlags_NoResize              |
                                 ImGuiWindowFlags_NoCollapse            |
                                 ImGuiWindowFlags_NoTitleBar            |
                                 ImGuiWindowFlags_NoScrollbar           |
                                 ImGuiWindowFlags_NoSavedSettings       |
                                 ImGuiWindowFlags_NoBringToFrontOnFocus;
    
    int win_width, win_height;
    SDL_GetWindowSize(window, &win_width, &win_height);

    glViewport(0.0, 0.0, win_width * 0.75, win_height - bar_height);

    ImGui::SetNextWindowSize(ImVec2(win_width * 0.25, (float)win_height - bar_height));
    ImGui::SetNextWindowPos(ImVec2(win_width* 0.75, 0.0 + bar_height));

    ImGui::Begin("ToolBar", nullptr, win_flags);
    {
        ImGui::Text("Check");
    }
    ImGui::End();
}

void App::render()
{
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

    // Mapping texture
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f, 1.0f); // top left
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f, 1.0f); // top right
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1.0f, 1.0f); // bottom right
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, 1.0f); // bootom left
    glEnd();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);
}

void App::destroy()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    glDeleteTextures(1, &gl_texture);
    SDL_GL_DeleteContext(gl_context);

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

    glGenTextures(1, &gl_texture);
    
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
