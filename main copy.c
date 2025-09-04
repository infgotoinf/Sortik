// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_sdlrenderer2.h"
#include <stdio.h>
#include <SDL.h>
#ifdef _WIN32
#include <windows.h>        // SetProcessDPIAware()
#endif

#include <SDL.h>
#include <SDL_syswm.h>

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

#include <stdio.h>
#include <string>

#include "fonts/ProggyVector.h"

#define DEVELOPER_OPTIONS // Disable this for release



void renderWithTransparency(SDL_Renderer* renderer, ImGuiIO& io, int transparent_colorref)
{
    ImGui::Render();
    SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, GetRValue(transparent_colorref), GetGValue(transparent_colorref), GetBValue(transparent_colorref), SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, NULL);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
}


//=================================================================================
//      START OF THE MAIN CODE
//=================================================================================

int main(int, char**)
{

//---------------------------------------------------------------------------------
//          SETUP
//---------------------------------------------------------------------------------

    // Lua intialisation
    // lua_init();

    // Setup SDL
    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with SDL_Renderer graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_BORDERLESS);
    SDL_Window* window = SDL_CreateWindow("Infinite Filter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1, 1, window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        SDL_Log("Error creating SDL_Renderer!");
        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImFontConfig config;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
    

    // Windows transparency setup
    static const COLORREF transparent_colorref = RGB(255, 0, 255);
#ifdef _WIN32
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
	HWND handle = wmInfo.info.win.window;
	if(!SetWindowLong(handle, GWL_EXSTYLE, GetWindowLong(handle, GWL_EXSTYLE) | WS_EX_LAYERED))
		fprintf(stderr, "SetWindowLong Error\n");
    if(!SetLayeredWindowAttributes(handle, transparent_colorref, 0, 1))
		fprintf(stderr, "SetLayeredWindowAttributes Error\n");
#endif


    // Load Fonts
    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
    builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
    builder.BuildRanges(&ranges);
    // io.Fonts->AddFontFromFileTTF("..\\imgui\\misc\\fonts\\ProggyVector.ttf", 16.0f, nullptr, ranges.Data);
    io.Fonts->AddFontFromMemoryCompressedTTF(ProggyVector_compressed_data, ProggyVector_compressed_size, 16.0f, nullptr, ranges.Data);

//---------------------------------------------------------------------------------
//          STYLE EDITING
//---------------------------------------------------------------------------------

    ImVec4* colors = ImGui::GetStyle().Colors;
    
    colors[ImGuiCol_TextDisabled]         = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
    colors[ImGuiCol_ChildBg]              = ImVec4(0.04f, 0.04f, 0.04f, 0.30f);
    colors[ImGuiCol_PopupBg]              = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_Border]               = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImGuiCol_FrameBg]              = ImVec4(0.50f, 0.50f, 0.50f, 0.30f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.64f, 0.64f, 0.64f, 0.50f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.78f, 0.78f, 0.78f, 0.70f);
    colors[ImGuiCol_TitleBg]              = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_MenuBarBg]            = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.02f, 0.02f, 0.02f, 0.52f);
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.31f, 0.31f, 0.31f, 0.61f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 0.71f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.51f, 0.51f, 0.51f, 0.81f);
    colors[ImGuiCol_CheckMark]            = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    colors[ImGuiCol_SliderGrab]           = ImVec4(0.78f, 0.78f, 0.78f, 0.70f);
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
    colors[ImGuiCol_Button]               = ImVec4(0.68f, 0.68f, 0.68f, 0.40f);
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.82f, 0.82f, 0.82f, 0.50f);
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.96f, 0.96f, 0.96f, 0.60f);
    colors[ImGuiCol_Header]               = ImVec4(0.50f, 0.50f, 0.50f, 0.30f);
    colors[ImGuiCol_HeaderHovered]        = ImVec4(0.64f, 0.64f, 0.64f, 0.50f);
    colors[ImGuiCol_HeaderActive]         = ImVec4(0.78f, 0.78f, 0.78f, 0.70f);
    colors[ImGuiCol_Separator]            = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImGuiCol_SeparatorHovered]     = ImVec4(0.82f, 0.82f, 0.82f, 0.00f);
    colors[ImGuiCol_SeparatorActive]      = ImVec4(0.96f, 0.96f, 0.96f, 0.00f);
    colors[ImGuiCol_ResizeGrip]           = ImVec4(1.00f, 1.00f, 1.00f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]    = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
    colors[ImGuiCol_ResizeGripActive]     = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_TabHovered]           = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
    colors[ImGuiCol_Tab]                  = ImVec4(0.50f, 0.50f, 0.50f, 0.40f);
    colors[ImGuiCol_TabSelected]          = ImVec4(0.78f, 0.78f, 0.78f, 0.80f);

    ImGuiStyle& style = ImGui::GetStyle();
    style.GrabRounding   = 3.0f;
    style.FrameRounding  = 3.0f;

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

    // Making app to take entire screen
    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);
    int display_width = dm.w;
    int display_height = dm.h; 
    SDL_SetWindowSize(window, display_width, display_height);
    SDL_SetWindowPosition(window, 0, 0);


    // Our state
    std::string filename = u8"..\\assets\\MyImage01.jpg";
    SDL_Texture* texture;
    int img_width, img_height, channels;
    bool ret = LoadTextureFromFile(filename.c_str(), renderer, &texture, &img_width, &img_height);

//=================================================================================
//      START OF THE MAIN LOOP
//=================================================================================

    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

//---------------------------------------------------------------------------------
//          WINDOW RAINBOW TRANSITION
//---------------------------------------------------------------------------------
        
        static float color_brightness     = 0.15f;
        static float old_color_brightness = color_brightness;
        static float color_change_speed   = 0.001f;
        static float r = color_brightness;
        static float g = 0.0f;
        static float b = 0.0f;

        if      (r >= color_brightness   &&                            b >  color_change_speed) b -= color_change_speed;
        else if (r >= color_brightness   && g <  color_brightness   && b <= color_change_speed) g += color_change_speed;
        else if (r >  color_change_speed && g >= color_brightness                             ) r -= color_change_speed;
        else if (r <= color_change_speed && g >= color_brightness   && b <  color_brightness  ) b += color_change_speed;
        else if (                           g >  color_change_speed && b >= color_brightness  ) g -= color_change_speed;
        else if (r <  color_brightness   && g <= color_change_speed && b >= color_brightness  ) r += color_change_speed;

        colors[ImGuiCol_WindowBg] = ImVec4(r, g, b, 1.0f);

//---------------------------------------------------------------------------------
//          MAIN WINDOW
//---------------------------------------------------------------------------------

        // static int f = 0;

        ImGuiWindowFlags main_window_flags = 0;
        main_window_flags |= ImGuiWindowFlags_NoScrollbar;  // Disable scrollbar
        main_window_flags |= ImGuiWindowFlags_MenuBar;      // Enable menu bar

        ImGui::Begin("Image Render", nullptr, main_window_flags);

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//          MENU BAR
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        if (ImGui::BeginMenuBar())
        {
            // File menu
            if (ImGui::BeginMenu(
                [&]() -> const char* {
                    switch (language){
                        case RUS: return u8"Файл";
                        case ENG:
                        default: return "File";
                    }
                }()))
            {
                ShowFileMenu();
                ImGui::EndMenu();
            }
            // // Edit menu
            // if (ImGui::BeginMenu(
            //     [&]() -> const char* {
            //         switch (language){
            //             case RUS: return u8"Правка";
            //             case ENG:
            //             default: return "Edit";
            //         }
            //     }()))
            // {
            //     ShowEditMenu();
            //     ImGui::EndMenu();
            // }
            // Filter menu
            if (ImGui::BeginMenu(
                [&]() -> const char* {
                    switch (language){
                        case RUS: return u8"Фильтр";
                        case ENG:
                        default: return "Filter";
                    }
                }()))
            {
                ShowFilterMenu(renderer, &texture);
                ImGui::EndMenu();
            }
            // Settings menu
            if (ImGui::BeginMenu(
                [&]() -> const char* {
                    switch (language){
                        case RUS: return u8"Настройки";
                        case ENG:
                        default: return "Settings";
                    }
                }()))
            {
                ShowSettingsMenu();
                ImGui::EndMenu();
            }
            // Help menu
            if (ImGui::BeginMenu(
                [&]() -> const char* {
                    switch (language){
                        case RUS: return u8"Помощь";
                        case ENG:
                        default: return "Help";
                    }
                }()))
            {
                ShowHelpMenu();
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//          MAIN WINDOW CONTENT
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        // ImGui::Text("pointer = %p", texture);
        // ImGui::Text("size = %d x %d", img_width, img_height);
        // ImGui::Text(filename.c_str());

        // ImGui::SliderInt("Image size", &f, -10, 10);
        // ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2((float)img_width, (float)img_height));
        

        ImVec2 image_window_size = ImVec2(img_width * (ImGui::GetWindowSize().y / img_height) - 60, ImGui::GetWindowSize().y - 60);

        ImGui::Image((ImTextureID)(intptr_t)texture, image_window_size);
        ImGui::End();

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//          CHILD WINDOWS
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        ImGui::SetNextWindowSize(ImVec2(500, 400));

    #ifdef DEVELOPER_OPTIONS
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
    #endif

        if (show_fd_window)
            drawGui(&filename, &img_width, &img_height, &texture, renderer, display_width, display_height);

        if (show_config_window)
        { // Configuratuion window
            ImGui::Begin("Configuration", &show_config_window, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::SeparatorText("Theme settings");
            if (ImGui::SliderFloat("Colour brightness", &color_brightness, 0.0f, 0.5f, "%.2f"))
            {
                if (r >= old_color_brightness) r = color_brightness;
                if (g >= old_color_brightness) g = color_brightness;
                if (b >= old_color_brightness) b = color_brightness;
                old_color_brightness = color_brightness;
            };
            if (ImGui::SliderFloat("Color change speed", &color_change_speed, 0.0f, (color_brightness / 10 > 0.1f ? 0.1f : color_brightness / 10)))
            { };
            if (color_change_speed > color_brightness / 10) color_change_speed = color_brightness / 10;
            ImGui::End();
        }
        if (show_credits_window)
        { // Credits window
            ImGui::Begin("Credits", &show_credits_window, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("...");
            ImGui::End();
        }

//---------------------------------------------------------------------------------
//          RENDERING
//---------------------------------------------------------------------------------

        renderWithTransparency(renderer, io, transparent_colorref);
    }

    if (texture) {
        SDL_DestroyTexture(texture);
    }

    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
