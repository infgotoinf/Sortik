# cmake -S . -B cbuild
# make -C cbuild
# rm -r cbuild
cmake_minimum_required(VERSION 3.12)

project(Sortik CXX)

set(CMAKE_CXX_STANDSRT 17)
set(CMAKE_CXX_STANDART_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

set(SOURCES
    main.cpp
)

add_compile_options(-Wall -Wextra)

add_executable(Sortik
    main.cpp

    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp

    imgui/backends/imgui_impl_sdl2.cpp
    imgui/backends/imgui_impl_sdlrenderer2.cpp

    imgui/implot/implot_demo.cpp
    imgui/implot/implot_items.cpp
    imgui/implot/implot.cpp
)

include_directories(AFTER
    imgui
    imgui/backends
    imgui/implot

    "C:/msys64/ucrt64/include/SDL2"
)

target_link_libraries(Sortik
    SDL2
)