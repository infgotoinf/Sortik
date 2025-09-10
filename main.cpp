// Dear ImGui: standalone example application for SDL2 + Vulkan

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

// Important note to the reader who wish to integrate imgui_impl_vulkan.cpp/.h in their own engine/app.
// - Common ImGui_ImplVulkan_XXX functions and structures are used to interface with imgui_impl_vulkan.cpp/.h.
//   You will use those if you want to use this rendering backend in your engine/app.
// - Helper ImGui_ImplVulkanH_XXX functions and structures are only used by this example (main.cpp) and by
//   the backend itself (imgui_impl_vulkan.cpp), but should PROBABLY NOT be used by your own engine/app code.
// Read comments in imgui_impl_vulkan.h.

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_sdlrenderer2.h"
#include "imgui/implot/implot.h"
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#include <SDL.h>
#include <SDL_syswm.h>
#ifdef _WIN32
#include <windows.h>        // SetProcessDPIAware()
#endif

#include <string>
#include <random>
#include <future>
#include <thread>

#define RENDER_WITH_TRANSPARENCY // Enable to make main window transparrent
//#define DEVELOPER_OPTIONS        // Disable this for release
#define SHOW_FPS

//=================================================================================
//      FUNCTIONS
//=================================================================================

void updateIntArray(const int number, int*& array)
{
    array = new int[number];
    for (int i = 0; i < number; i++) array[i] = i;
}

void copyPasteArray(const int number, int*& copy, int*& paste)
{
    paste = new int[number];
    for (int i = 0; i < number; i++) {
        paste[i] = copy[i];
    }
}

std::random_device dev;
std::mt19937 rnd(dev());

void shuffleIntArray(const int number, int*& array)
{
    for (int i = number - 1; i > 0; i--)
    {
        std::uniform_int_distribution<int> dist(0, i);
        int j = dist(rnd);
        std::swap(array[i], array[j]);
    }
}

bool verifyArrayIsSorted(int*& array, const int number)
{
    for (int i = 0; i < number; i++)
    {
        if (array[i] != i)
            return 0;
    }
    return 1;
}

//---------------------------------------------------------------------------------
//      SORTS
//---------------------------------------------------------------------------------

std::mutex numbers_mutex;

//------BOGO-----------------------------------------------------------------------
std::mutex bogo_numbers_mutex;
std::future<void> bogo_future;
std::atomic<int> bogo_iterations(0);
std::atomic<double> bogo_sort_time_ms(0.0);
std::chrono::time_point<std::chrono::high_resolution_clock> bogo_start_time;

void bogoSort(int* array, const int number, std::atomic<int>& iter_count, std::atomic<double>& sort_time_ms)
{
    iter_count = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    while (true) {
        iter_count++;
        {
            std::lock_guard<std::mutex> lock(bogo_numbers_mutex);
            if (verifyArrayIsSorted(array, number))
                break;
            shuffleIntArray(number, array);
        }
        std::this_thread::yield();
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    sort_time_ms = duration.count();
}

//------SHELL----------------------------------------------------------------------
std::mutex shell_numbers_mutex;
std::future<void> shell_future;
std::atomic<int> shell_operations(0);
std::atomic<double> shell_sort_time_ms(0.0);
std::chrono::time_point<std::chrono::high_resolution_clock> shell_start_time;

void shellSort(int* array, const int number, std::atomic<int>& oper_count, std::atomic<double>& sort_time_ms)
{
    oper_count = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Start with a big gap, then reduce the gap
    for (int gap = number/2; gap > 0; gap /= 2)
    {
        // Do a gapped insertion sort for this gap size.
        // The first gap elements a[0..gap-1] are already in gapped order
        // keep adding one more element until the entire array is
        // gap sorted 
        for (int i = gap; i < number; i += 1)
        {
            // add a[i] to the elements that have been gap sorted
            // save a[i] in temp and make a hole at position i
            int temp = array[i];

            // shift earlier gap-sorted elements up until the correct 
            // location for a[i] is found
            int j;
            
            for (j = i; j >= gap && array[j - gap] > temp; j -= gap)
            {
                array[j] = array[j - gap];
                oper_count++;
            }

            //  put temp (the original a[i]) in its correct location
            array[j] = temp;
            oper_count++;
            
            std::this_thread::yield();
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    sort_time_ms = duration.count();
}

//------RADIX----------------------------------------------------------------------
std::mutex radix_numbers_mutex;
std::future<void> radix_future;
std::atomic<int> radix_operations(0);
std::atomic<double> radix_sort_time_ms(0.0);
std::chrono::time_point<std::chrono::high_resolution_clock> radix_start_time;

int getMax(int* arr, int n)
{
    int mx = arr[0];
    for (int i = 1; i < n; i++) {
        if (arr[i] > mx)
            mx = arr[i];
        radix_operations++;
    }
    return mx;
}

// A function to do counting sort of arr[]
// according to the digit
// represented by exp.
void countSort(int* arr, int n, int exp, std::atomic<int>& oper_count)
{
    // Use dynamic allocation instead of VLA (Variable Length Array)
    int* output = new int[n];
    int count[10] = { 0 };

    // Store count of occurrences
    for (int i = 0; i < n; i++) {
        count[(arr[i] / exp) % 10]++;
        oper_count++;
    }

    // Change count[i] to contain actual position
    for (int i = 1; i < 10; i++) {
        count[i] += count[i - 1];
        oper_count++;
    }

    // Build the output array
    for (int i = n - 1; i >= 0; i--) {
        output[count[(arr[i] / exp) % 10] - 1] = arr[i];
        count[(arr[i] / exp) % 10]--;
        oper_count++;
    }

    // Copy the output array to arr[]
    for (int i = 0; i < n; i++) {
        arr[i] = output[i];
        oper_count++;
    }
    
    delete[] output;
}

void radixSort(int* arr, int n, std::atomic<int>& oper_count, std::atomic<double>& sort_time_ms)
{
    oper_count = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    // Find the maximum number to know number of digits
    int m = getMax(arr, n);

    // Do counting sort for every digit
    for (int exp = 1; m / exp > 0; exp *= 10) {
        {
            std::lock_guard<std::mutex> lock(radix_numbers_mutex);
            countSort(arr, n, exp, oper_count);
        }

        std::this_thread::yield();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    sort_time_ms = duration.count();
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

std::vector<int> downsampleArray(const int* data, int original_size) {
    int max_points = 100000;
    std::vector<int> downsampled;
    if (original_size <= max_points) {
        // No need to downsample
        downsampled.assign(data, data + original_size);
        return downsampled;
    }
    
    // Calculate sampling rate
    int step = original_size / max_points;
    if (step < 1) step = 1;
    
    // Sample the data
    downsampled.reserve(max_points);
    for (int i = 0; i < original_size; i += step) {
        downsampled.push_back(data[i]);
    }
    
    return downsampled;
}

#ifdef DEVELOPER_OPTIONS
std::string debug_array(const int* array, const int number)
{
    std::string info;
    for (int i = 0; i < number; i++)
    {
        info += std::to_string(array[i]) + " ";
    }
    return info;
}
#endif

//=================================================================================
//      SDL SETUP
//=================================================================================

void renderWindow(SDL_Renderer* renderer, ImGuiIO& io, int transparent_colorref = 0)
{
    ImGui::Render();
    SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
#ifdef RENDER_WITH_TRANSPARENCY
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

//---------------------------------------------------------------------------------
//      START OF THE MAIN CODE
//---------------------------------------------------------------------------------

int main(int, char**)
{
    // Setup SDL
#ifdef _WIN32
    ::SetProcessDPIAware();
#endif
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
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1, 1, window_flags);
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
#if defined(_WIN32) && defined(RENDER_WITH_TRANSPARENCY)
    static const COLORREF transparent_colorref = RGB(255, 0, 255);
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
	HWND handle = wmInfo.info.win.window;
	if(!SetWindowLong(handle, GWL_EXSTYLE, GetWindowLong(handle, GWL_EXSTYLE) | WS_EX_LAYERED))
		fprintf(stderr, "SetWindowLong Error\n");
    if(!SetLayeredWindowAttributes(handle, transparent_colorref, 0, 1))
		fprintf(stderr, "SetLayeredWindowAttributes Error\n");
#endif

//---------------------------------------------------------------------------------
//          STYLE EDITING
//---------------------------------------------------------------------------------

    ImVec4* gui_colors = ImGui::GetStyle().Colors;
    
    gui_colors[ImGuiCol_TextDisabled]         = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
    gui_colors[ImGuiCol_ChildBg]              = ImVec4(0.04f, 0.04f, 0.04f, 0.30f);
    gui_colors[ImGuiCol_PopupBg]              = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    gui_colors[ImGuiCol_Border]               = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    gui_colors[ImGuiCol_FrameBg]              = ImVec4(0.50f, 0.50f, 0.50f, 0.30f);
    gui_colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.64f, 0.64f, 0.64f, 0.50f);
    gui_colors[ImGuiCol_FrameBgActive]        = ImVec4(0.78f, 0.78f, 0.78f, 0.70f);
    gui_colors[ImGuiCol_TitleBg]              = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    gui_colors[ImGuiCol_TitleBgActive]        = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    gui_colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    gui_colors[ImGuiCol_MenuBarBg]            = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    gui_colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.02f, 0.02f, 0.02f, 0.52f);
    gui_colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.31f, 0.31f, 0.31f, 0.61f);
    gui_colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 0.71f);
    gui_colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.51f, 0.51f, 0.51f, 0.81f);
    gui_colors[ImGuiCol_CheckMark]            = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    gui_colors[ImGuiCol_SliderGrab]           = ImVec4(0.78f, 0.78f, 0.78f, 0.70f);
    gui_colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
    gui_colors[ImGuiCol_Button]               = ImVec4(0.68f, 0.68f, 0.68f, 0.40f);
    gui_colors[ImGuiCol_ButtonHovered]        = ImVec4(0.82f, 0.82f, 0.82f, 0.50f);
    gui_colors[ImGuiCol_ButtonActive]         = ImVec4(0.96f, 0.96f, 0.96f, 0.60f);
    gui_colors[ImGuiCol_Header]               = ImVec4(0.50f, 0.50f, 0.50f, 0.30f);
    gui_colors[ImGuiCol_HeaderHovered]        = ImVec4(0.64f, 0.64f, 0.64f, 0.50f);
    gui_colors[ImGuiCol_HeaderActive]         = ImVec4(0.78f, 0.78f, 0.78f, 0.70f);
    gui_colors[ImGuiCol_Separator]            = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    gui_colors[ImGuiCol_SeparatorHovered]     = ImVec4(0.82f, 0.82f, 0.82f, 0.00f);
    gui_colors[ImGuiCol_SeparatorActive]      = ImVec4(0.96f, 0.96f, 0.96f, 0.00f);
    gui_colors[ImGuiCol_ResizeGrip]           = ImVec4(1.00f, 1.00f, 1.00f, 0.20f);
    gui_colors[ImGuiCol_ResizeGripHovered]    = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
    gui_colors[ImGuiCol_ResizeGripActive]     = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    gui_colors[ImGuiCol_TabHovered]           = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
    gui_colors[ImGuiCol_Tab]                  = ImVec4(0.50f, 0.50f, 0.50f, 0.40f);
    gui_colors[ImGuiCol_TabSelected]          = ImVec4(0.78f, 0.78f, 0.78f, 0.80f);

    ImGuiStyle& gui_style = ImGui::GetStyle();
    gui_style.GrabRounding   = 3.0f;
    gui_style.FrameRounding  = 3.0f;

    // gui_style.ScaleAllSizes(main_scale);
    // gui_style.FontScaleDpi = main_scale;

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
    //ImVec4* plot_colors = ImPlot::GetStyle().Colors;
    
    // plot_colors[ImPlotCol_Line]          = IMPLOT_AUTO_COL;
    // plot_colors[ImPlotCol_Fill]          = IMPLOT_AUTO_COL;
    // plot_colors[ImPlotCol_MarkerOutline] = IMPLOT_AUTO_COL;
    // plot_colors[ImPlotCol_MarkerFill]    = IMPLOT_AUTO_COL;
    // plot_colors[ImPlotCol_ErrorBar]      = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    // plot_colors[ImPlotCol_FrameBg]       = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    // plot_colors[ImPlotCol_PlotBg]        = ImVec4(0.92f, 0.92f, 0.95f, 1.00f);
    // plot_colors[ImPlotCol_PlotBorder]    = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    // plot_colors[ImPlotCol_LegendBg]      = ImVec4(0.92f, 0.92f, 0.95f, 1.00f);
    // plot_colors[ImPlotCol_LegendBorder]  = ImVec4(0.80f, 0.81f, 0.85f, 1.00f);
    // plot_colors[ImPlotCol_LegendText]    = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    // plot_colors[ImPlotCol_TitleText]     = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    // plot_colors[ImPlotCol_InlayText]     = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    // plot_colors[ImPlotCol_AxisText]      = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    // plot_colors[ImPlotCol_AxisGrid]      = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    // plot_colors[ImPlotCol_AxisBgHovered] = ImVec4(0.92f, 0.92f, 0.95f, 1.00f);
    // plot_colors[ImPlotCol_AxisBgActive]  = ImVec4(0.92f, 0.92f, 0.95f, 0.75f);
    // plot_colors[ImPlotCol_Selection]     = ImVec4(1.00f, 0.65f, 0.00f, 1.00f);
    // plot_colors[ImPlotCol_Crosshairs]    = ImVec4(0.23f, 0.10f, 0.64f, 0.50f);

    ImPlotStyle& plot_style = ImPlot::GetStyle();
    plot_style.LineWeight       = 1.5;
    plot_style.Marker           = ImPlotMarker_None;
    plot_style.MarkerSize       = 4;
    plot_style.MarkerWeight     = 1;
    plot_style.FillAlpha        = 1.0f;
    plot_style.ErrorBarSize     = 5;
    plot_style.ErrorBarWeight   = 1.5f;
    plot_style.DigitalBitHeight = 8;
    plot_style.DigitalBitGap    = 4;
    plot_style.PlotBorderSize   = 0;
    plot_style.MinorAlpha       = 1.0f;
    plot_style.MajorTickLen     = ImVec2(0,0);
    plot_style.MinorTickLen     = ImVec2(0,0);
    plot_style.MajorTickSize    = ImVec2(0,0);
    plot_style.MinorTickSize    = ImVec2(0,0);
    plot_style.MajorGridSize    = ImVec2(1.2f,1.2f);
    plot_style.MinorGridSize    = ImVec2(1.2f,1.2f);
    plot_style.PlotPadding      = ImVec2(12,12);
    plot_style.LabelPadding     = ImVec2(5,5);
    plot_style.LegendPadding    = ImVec2(5,5);
    plot_style.MousePosPadding  = ImVec2(5,5);
    plot_style.PlotMinSize      = ImVec2(300,225);

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

#ifdef RENDER_WITH_TRANSPARENCY
    // Making app to take entire screen
    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);
    int display_width = dm.w;
    int display_height = dm.h; 
    SDL_SetWindowSize(window, display_width, display_height);
    SDL_SetWindowPosition(window, 0, 0);
#endif

//---------------------------------------------------------------------------------
//      BEGINNING STATE
//---------------------------------------------------------------------------------

#ifdef DEVELOPER_OPTIONS
    bool show_demo_window = false;
    bool show_impot_demo_window = false;
#endif

    bool show_shellsort_window = false;
    bool show_radixsort_window = false;
    bool show_bogosort_window = false;
    bool render_charts = true;


    int number_of_numbers = 1000;
    int* numbers;
    int* shell_numbers;
    int* radix_numbers;
    int* bogo_numbers;
    updateIntArray(number_of_numbers, numbers);
    copyPasteArray(number_of_numbers, numbers, shell_numbers);
    copyPasteArray(number_of_numbers, numbers, radix_numbers);
    copyPasteArray(number_of_numbers, numbers, bogo_numbers);

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
            ImGui::Begin("Sortik", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

            if (ImGui::Button("Shuffle"))
            {
                shuffleIntArray(number_of_numbers, numbers);
                copyPasteArray(number_of_numbers, numbers, shell_numbers);
                copyPasteArray(number_of_numbers, numbers, radix_numbers);
                copyPasteArray(number_of_numbers, numbers, bogo_numbers);
            }
            ImGui::SameLine();
            if (ImGui::Button("Beggin Sort"))
            {
                if (show_shellsort_window)
                    if (!shell_future.valid() || shell_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        std::lock_guard<std::mutex> lock(shell_numbers_mutex);
                        shell_operations = 0;
                        shell_sort_time_ms = 0.0;
                        shell_start_time = std::chrono::high_resolution_clock::now();
                        shell_future = std::async(std::launch::async, shellSort, 
                                                shell_numbers, number_of_numbers, 
                                                std::ref(shell_operations), 
                                                std::ref(shell_sort_time_ms));
                    }
                if (show_radixsort_window)
                    if (!radix_future.valid() || radix_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        std::lock_guard<std::mutex> lock(radix_numbers_mutex);
                        shuffleIntArray(number_of_numbers, radix_numbers);
                        radix_operations = 0;
                        radix_sort_time_ms = 0.0;
                        radix_start_time = std::chrono::high_resolution_clock::now();
                        radix_future = std::async(std::launch::async, radixSort, 
                                                radix_numbers, number_of_numbers, 
                                                std::ref(radix_operations), 
                                                std::ref(radix_sort_time_ms));
                    }
                if (show_bogosort_window)
                    if (!bogo_future.valid() || bogo_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        std::lock_guard<std::mutex> lock(bogo_numbers_mutex);
                        bogo_iterations = 0;
                        bogo_sort_time_ms = 0.0;
                        bogo_start_time = std::chrono::high_resolution_clock::now();
                        bogo_future = std::async(std::launch::async, bogoSort, 
                                                bogo_numbers, number_of_numbers, 
                                                std::ref(bogo_iterations), 
                                                std::ref(bogo_sort_time_ms));
                    }

            }
            
            ImGui::Text("Ctrl + left-click on the slider to input any number");
            if (ImGui::SliderInt("Number of numbers", &number_of_numbers, 100, 10000, nullptr, ImGuiSliderFlags_NoRoundToFormat))
            {
                updateIntArray(number_of_numbers, numbers);
                copyPasteArray(number_of_numbers, numbers, shell_numbers);
                copyPasteArray(number_of_numbers, numbers, radix_numbers);
                copyPasteArray(number_of_numbers, numbers, bogo_numbers);
            }

            ImGui::SeparatorText("Shell Sort");
            ImGui::Checkbox("Do", &show_shellsort_window);
            if (shell_future.valid())
            {
                 auto status = shell_future.wait_for(std::chrono::seconds(0));
                 int operations = shell_operations.load();

                if (status == std::future_status::ready)
                {
                    double time_seconds = shell_sort_time_ms.load() / 1000.0;
                    ImGui::Text("Time: %.2f sec, Operations: %d", time_seconds, operations);
                }
                else
                {
                    auto current_time = std::chrono::high_resolution_clock::now();
                    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    current_time - shell_start_time).count();
                    ImGui::Text("Time: %.2f sec, Operations: %d", elapsed_ms / 1000.0, operations);
                }
            }
            ImGui::SeparatorText("Radix Sort");
            ImGui::Checkbox("Do##2", &show_radixsort_window);
            if (radix_future.valid())
            {
                auto status = radix_future.wait_for(std::chrono::seconds(0));
                int operations = radix_operations.load();

                if (status == std::future_status::ready)
                {
                    double time_seconds = radix_sort_time_ms.load() / 1000.0;
                    ImGui::Text("Time: %.2f sec, Operations: %d", time_seconds, operations);
                }
                else
                {
                    auto current_time = std::chrono::high_resolution_clock::now();
                    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    current_time - radix_start_time).count();
                    ImGui::Text("Time: %.2f sec, Operations: %d", elapsed_ms / 1000.0, operations);
                }
            }
            ImGui::SeparatorText("Bogo Sort");
            ImGui::Checkbox("Do##3", &show_bogosort_window);
            if (bogo_future.valid())
            {
                 auto status = bogo_future.wait_for(std::chrono::seconds(0));
                 int iterations = bogo_iterations.load();

                if (status == std::future_status::ready)
                {
                    double time_seconds = bogo_sort_time_ms.load() / 1000.0;
                    ImGui::Text("Time: %.2f sec, Iterations: %d", time_seconds, iterations);
                }
                else
                {
                    auto current_time = std::chrono::high_resolution_clock::now();
                    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    current_time - bogo_start_time).count();
                    ImGui::Text("Time: %.2f sec, Iterations: %d", elapsed_ms / 1000.0, iterations);
                }
            }
            ImGui::Separator();
            ImGui::Checkbox("Render charts", &render_charts);

        #ifdef DEVELOPER_OPTIONS
            ImGui::Checkbox("ImGui Demo Window", &show_demo_window);
            ImGui::SameLine();
            ImGui::Checkbox("Implot Demo Window", &show_impot_demo_window);
            //ImGui::Text(debug_array(numbers, number_of_numbers).c_str());
        #endif
        #ifdef SHOW_FPS
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        #endif

            ImGui::End();
        }

//---------------------------------------------------------------------------------
//          SORT WINDOWS
//---------------------------------------------------------------------------------
        if (show_shellsort_window || show_radixsort_window || show_bogosort_window)
        {
            ImGui::Begin("Sort Window", nullptr, ImGuiWindowFlags_NoScrollbar);
            if (render_charts)
            {
                std::lock_guard<std::mutex> lock(numbers_mutex);
                ImVec2 pivot_window_size = ImVec2(ImGui::GetWindowSize().x - 15, ImGui::GetWindowSize().y - 50);
                if (ImPlot::BeginPlot("My Plot", pivot_window_size)) {
                    if (show_shellsort_window)
                    {
                        auto downsampled = downsampleArray(shell_numbers, number_of_numbers);
                        ImPlot::PlotBars("Shellsort", downsampled.data(), downsampled.size());
                    }
                    if (show_radixsort_window)
                    {
                        auto downsampled = downsampleArray(radix_numbers, number_of_numbers);
                        ImPlot::PlotBars("Radix Sort", downsampled.data(), downsampled.size());
                    }
                    if (show_bogosort_window)
                    {
                        auto downsampled = downsampleArray(bogo_numbers, number_of_numbers);
                        ImPlot::PlotBars("Bogosort", downsampled.data(), downsampled.size());
                    }
                    ImPlot::EndPlot();
                    ImGui::Text("I recomend right-clicking the chart and X-Y-Axis auto-fitting");
                }
            }
            ImGui::End();
        }
  
//---------------------------------------------------------------------------------
//          RENDERING
//---------------------------------------------------------------------------------   
    #ifdef RENDER_WITH_TRANSPARENCY
        renderWindow(renderer, io, transparent_colorref);   
    #else
        renderWindow(renderer, io);
    #endif
    }

    // Cleanup
    delete[] numbers;
    delete[] shell_numbers;
    delete[] radix_numbers;
    delete[] bogo_numbers;

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
