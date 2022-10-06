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
    bool quit = false;

    const u64 fps = 120 / 1000; // FPS in ms
    Uint64 elapsed = 0;
    Uint64 frame_time = 0;

    // Debug
    loadBMP("bmp_examples/road.bmp");

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

void App::update()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    
    int win_width, win_height;
    SDL_GetWindowSize(window, &win_width, &win_height);

    char image_path[255] = {0};
    float bar_height = 0.0;
    if (ImGui::BeginMainMenuBar()) {
        bar_height = ImGui::GetWindowHeight();

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open")) { /* Do stuff */ }
            if (ImGui::MenuItem("Save")) { saveBMP(); }
            if (ImGui::MenuItem("Close")) {
                glDeleteTextures(1, &gl_texture);
                image_surface = NULL;
                current_image_path[0] = '\0';
            }
            ImGui::EndMenu();
        }
        
        ImGui::Separator();

        // Path/filename text
        static const float max_path_w = win_width * 0.25;
        std::string display_text;
        if (current_image_path[0]) {
            display_text = current_image_path;
            while(ImGui::CalcTextSize(display_text.c_str()).x > max_path_w) {
                display_text.pop_back();
            }

            if (display_text != current_image_path) {
                display_text += "..";
            }
        } else {
            display_text = "Enter image path: ";
        }

        ImGui::Text("%s", display_text.c_str());
        ImGui::Dummy(ImVec2(max_path_w -
                     ImGui::CalcTextSize(display_text.c_str()).x, 0));

        // ImGui::Separator();
    

        ImGui::PushItemWidth(win_width);
        if (ImGui::InputText("##path_textbox", image_path, sizeof(image_path),
            ImGuiInputTextFlags_EnterReturnsTrue))
        {
            if (!loadBMP(image_path)) {
                printf("Unable to load image %s! SDL_Error: %s\n",
                        image_path, SDL_GetError());
            }
        }
        ImGui::PopItemWidth();

        ImGui::EndMainMenuBar();
    }

    ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoMove                |
                                 ImGuiWindowFlags_NoResize              |
                                 ImGuiWindowFlags_NoCollapse            |
                                 ImGuiWindowFlags_NoTitleBar            |
                                 ImGuiWindowFlags_NoScrollbar           |
                                 ImGuiWindowFlags_NoSavedSettings       |
                                 ImGuiWindowFlags_NoBringToFrontOnFocus;
    

    glViewport(0.0, 0.0, win_width * 0.75, win_height - bar_height);

    ImGui::SetNextWindowSize(ImVec2(win_width * 0.25, (float)win_height - bar_height));
    ImGui::SetNextWindowPos(ImVec2(win_width* 0.75, 0.0 + bar_height));

    if (!current_image_path[0]) { return; }
    ImGui::Begin("ToolBar", nullptr, win_flags);
    {
        float cur_win_width = ImGui::GetWindowWidth();
        float text_indent = ImGui::CalcTextSize("ToolBar").x;

        ImGui::SetCursorPosX((cur_win_width - text_indent) * 0.5);
        ImGui::TextColored({0, 255, 0, 255}, "Tool Bar");

        if (ImGui::Button("Grayscale image")) {
            grayscaleBMP();
        }
        
        static int brightness = 0;
        static int prev_brightness = 0;
        if (ImGui::SliderInt("Brightness", &brightness, -30.f, 30.f)) {
            adjustBrightness(brightness - prev_brightness);
            prev_brightness = brightness;
        }

    }
    ImGui::End();
}

void App::render()
{
    if (image_surface) {
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

        // Mapping texture
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f, 1.0f); // top left
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f, 1.0f); // top right
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1.0f, 1.0f); // bottom right
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, 1.0f); // bootom left
        glEnd();
    } else {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

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
    strncpy(current_image_path, path, 255);

    SDL_Surface *temp = SDL_LoadBMP(path);

    if (!temp) {
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

bool App::saveBMP()
{

}

void App::grayscaleBMP()
{
    image_surface = SDL_ConvertSurfaceFormat(image_surface, SDL_PIXELFORMAT_ARGB8888, 0);
    Uint32 *pixels = (Uint32 *)image_surface->pixels;

    Uint8 r = 0, g = 0, b = 0, v = 0;
    for (int y = 0; y < image_surface->h; y++) {
      for (int x = 0; x < image_surface->w; x++) {
         Uint32 pixel = pixels[y * image_surface->w + x];

         SDL_GetRGB(pixel, image_surface->format, &r, &g, &b);
         v = 0.212671f * r + 0.715160f * g + 0.072169f * b;
         pixel = SDL_MapRGB(image_surface->format, v, v, v);

         pixels[y * image_surface->w + x] = pixel;
      }
    }
}

int truncate(int value)
{
    if (value < 0)   return 0;
    if (value > 255) return 255;

    return value;
}

void App::adjustBrightness(int brightness)
{
    Uint32 *pixels = (Uint32 *)image_surface->pixels;
    Uint8 r = 0, g = 0, b = 0;

    for (int y = 0; y < image_surface->h; y++) {
      for (int x = 0; x < image_surface->w; x++) {
         Uint32 pixel = pixels[y * image_surface->w + x];

         SDL_GetRGB(pixel, image_surface->format, &r, &g, &b);
         r = truncate(r + brightness);
         g = truncate(g + brightness);
         b = truncate(b + brightness);
         pixel = SDL_MapRGB(image_surface->format, r, g, b);

         pixels[y * image_surface->w + x] = pixel;
      }
    }
}

