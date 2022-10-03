#include "Gui.h"
#include "imgui_impl_opengl3.h"

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

void Gui::init()
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

void Gui::update()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    
    float bar_height = 0.0;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open..")) { /* Do stuff */ }
            if (ImGui::MenuItem("Save"))   { /* Do stuff */ }
            if (ImGui::MenuItem("Close"))  { /* Do stuff */ }
            ImGui::EndMenu();
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

void Gui::render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Gui::cleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
}
