// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_sdlrenderer2.h"
#include "imgui/implot/implot.h"
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

//#define ENABLE_CUSOM_FONT     // Enable for custom font
//#define RUN_WITH_TRANSPARENCY // Enable to make main window transparrent
#define DEVELOPER_OPTIONS     // Disable this for release

#ifdef ENABLE_CUSOM_FONT
#include "fonts/ProggyVector.h"
#endif


void renderWindow(SDL_Renderer* renderer, ImGuiIO& io, int transparent_colorref)
{
    ImGui::Render();
    SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
#ifdef RUN_WITH_TRANSPARENCY
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, GetRValue(transparent_colorref), GetGValue(transparent_colorref), GetBValue(transparent_colorref), SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, NULL);
#else
    SDL_SetRenderDrawColor(renderer, (Uint8)(155), (Uint8)(155), (Uint8)(155), (Uint8)(255));
#endif
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

    // Setup SDL
#ifdef _WIN32
    ::SetProcessDPIAware();
#endif
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with SDL_Renderer graphics context
    float main_scale = ImGui_ImplSDL2_GetContentScaleForDisplay(0);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)(1280 * main_scale), (int)(720 * main_scale), window_flags);
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
    //SDL_RendererInfo info;
    //SDL_GetRendererInfo(renderer, &info);
    //SDL_Log("Current SDL_Renderer: %s", info.name);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Windows transparency setup
    static const COLORREF transparent_colorref = RGB(255, 0, 255);
#if defined(_WIN32) && defined(RUN_WITH_TRANSPARENCY)
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
	HWND handle = wmInfo.info.win.window;
	if(!SetWindowLong(handle, GWL_EXSTYLE, GetWindowLong(handle, GWL_EXSTYLE) | WS_EX_LAYERED))
		fprintf(stderr, "SetWindowLong Error\n");
    if(!SetLayeredWindowAttributes(handle, transparent_colorref, 0, 1))
		fprintf(stderr, "SetLayeredWindowAttributes Error\n");
#endif


#ifdef ENABLE_CUSOM_FONT
    // Load Fonts
    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
    builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
    builder.BuildRanges(&ranges);
    // io.Fonts->AddFontFromFileTTF("..\\imgui\\misc\\fonts\\ProggyVector.ttf", 16.0f, nullptr, ranges.Data);
    io.Fonts->AddFontFromMemoryCompressedTTF(ProggyVector_compressed_data, ProggyVector_compressed_size, 16.0f, nullptr, ranges.Data);
#endif

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

    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

#ifdef RUN_WITH_TRANSPARENCY
    // Making app to take entire screen
    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);
    int display_width = dm.w;
    int display_height = dm.h; 
    SDL_SetWindowSize(window, display_width, display_height);
    SDL_SetWindowPosition(window, 0, 0);
#endif

    // Our state
    bool show_demo_window = true;
    bool show_impot_demo_window = true;
    bool show_shellsort = false;
    bool show_radixsort = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


    int num = 1000;
    int numbers[11] = {1,2,3,4,5,6,7,8,9,10,11};

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

    #ifdef DEVELOPER_OPTIONS
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
        if (show_impot_demo_window)
            ImPlot::ShowDemoWindow(&show_impot_demo_window);
    #endif

//---------------------------------------------------------------------------------
//          MAIN WINDOW
//---------------------------------------------------------------------------------

        {
            ImGui::Begin("Sortik");

            ImGui::Button("Shuffle");
            ImGui::SliderInt("Number of numbers", &num, 100, 10000, nullptr, ImGuiSliderFlags_NoRoundToFormat);
            ImGui::Checkbox("Shellsort", &show_shellsort);
            ImGui::Checkbox("Radix Sort", &show_radixsort);
            #ifdef DEVELOPER_OPTIONS
            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::Checkbox("Implot demo Window", &show_impot_demo_window);
            #endif

            ImGui::End();
        }

//---------------------------------------------------------------------------------
//          SORT WINDOWS
//---------------------------------------------------------------------------------
        if (show_shellsort || show_radixsort)
        {
            ImGui::Begin("Sort Window", nullptr, ImGuiWindowFlags_NoScrollbar);

            ImVec2 pivot_window_size = ImVec2(ImGui::GetWindowSize().x - 15, ImGui::GetWindowSize().y - 35);
            if (ImPlot::BeginPlot("My Plot", pivot_window_size)) {
                if (show_shellsort)
                    ImPlot::PlotBars("Shellsort", numbers, num, 1, 0, 0, 0, 1);
                if (show_radixsort)
                    ImPlot::PlotBars("Radix Sort", numbers, num, 1, 0, 0, 0, 1);
                ImPlot::EndPlot();
            }

            ImGui::End();
        }
  
//---------------------------------------------------------------------------------
//          RENDERING
//---------------------------------------------------------------------------------   

        renderWindow(renderer, io, transparent_colorref);     
    }

    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}