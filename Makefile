##---------------------------------------------------------------------------------
#
#                      ██████╗ ██╗   ██╗██╗██╗     ██████╗
#                      ██╔══██╗██║   ██║██║██║     ██╔══██╗
#                      ██████╔╝██║   ██║██║██║     ██║  ██║
#                      ██╔══██╗██║   ██║██║██║     ██║  ██║
#                      ██████╔╝╚██████╔╝██║███████╗██████╔╝
#                      ╚═════╝  ╚═════╝ ╚═╝╚══════╝╚═════╝
#
##---------------------------------------------------------------------------------
##                !THIS FROJECT IS CONFIGURED TO RUN IN VS CODE!
##
## Making a build takes time, so launch.json may throw an error at the first run
##      or you will have to rerun build to see your updated application!
## - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
##
## Go to "Run and Debug" (Ctrl+Shift+D) and choose "Debug Build" or "Release Build"
## To clean "build" folder press (Ctrl+Shift+P) -> "Tasks: Run Task" -> make-clean
##
##---------------------------------------------------------------------------------
#                           Requirements to build
#
# To build this programm you will need: make, g++, libstdc++, lwinpthread, bzip2
# You need to download MSYS2 (https://www.msys2.org/)
# And install them via UCRT64 (C:\msys64\ucrt64.exe):
# pacman -S make
# pacman -S mingw-w64-ucrt-x86_64-toolchain
#
# You will also need SDL2
# pacman -S mingw-w64-ucrt-x86_64-SDL2
# 
#----------------------------------------------------------------------------------
##           If you prefer to use console or don't want to use VS Code
##
## in UCRT64 cd to the project folder and run:
## make build=debug
##        OR
## make build=release
## 
## Use 'make clean' to clean build folder
##
##---------------------------------------------------------------------------------
#                              For developers
#
# After building a release I compress it via UPX https://github.com/upx/upx
#
#----------------------------------------------------------------------------------
#
# vulkan1.dll

CXX  = g++

RELEASE = RELEASE_Sortik
DEBUG   = DEBUG_Sortik

SRC_DIR       = ./
BUILD_DIR     = ./build
IMGUI_DIR     = ./imgui
BACKENDS_DIR  = $(IMGUI_DIR)/backends
IMPLOT_DIR    = $(IMGUI_DIR)/implot
SDL2_DIR      = C:/msys64/ucrt64/include/SDL2


SOURCES := main.cpp \
           $(shell find $(IMGUI_DIR) -name '*.cpp')

SOURCES := $(basename $(notdir $(SOURCES)))
OBJS    := $(SOURCES:%=$(BUILD_DIR)/$(build)_%.o)


CXXFLAGS = -std=c++17 \
           -I$(IMGUI_DIR) \
           -I$(BACKENDS_DIR) \
           -I$(IMPLOT_DIR) \
           -I$(SDL2_DIR)

RELEASE_CXXFLAGS = -g0 -O3 -w -DNDEBUG -flto -fno-rtti -fno-exceptions \
                   -ffunction-sections -fdata-sections -Wl,--gc-sections \
                   -fvisibility=hidden -fomit-frame-pointer -funroll-loops \
                   -fstrict-aliasing -fipa-pta -ftree-vectorize \
                   -fno-semantic-interposition -Wl,-O3 -Wl,--relax \
                   -Wl,--strip-all -mfpmath=both -mbranch-cost=2 \
                   -fno-stack-protector -fno-unwind-tables
                   # There are hell-a-lot-of stuff

DEBUG_CXXFLAGS = -g -g3 -O0 -Wall -Wextra -pedantic

LDFLAGS = -lmingw32 -lSDL2main -lSDL2 \
          -lrpcrt4 -Wl,--dynamicbase -Wl,--nxcompat \
          -static-libstdc++ -static-libgcc -static -lwinpthread -lsetupapi -lhid \
          -lwinmm -limm32 -lshell32 -lole32 -loleaut32 -luuid -lversion -msse2

##---------------------------------------------------------------------------------
##                         DEBUG AND RELEASE REALISATION
##---------------------------------------------------------------------------------

ifeq ($(build), debug)
	CXXFLAGS += $(DEBUG_CXXFLAGS)
	EXE = $(DEBUG)
endif
ifeq ($(build), release)
	CXXFLAGS += $(RELEASE_CXXFLAGS)
	EXE = $(RELEASE)
endif

##---------------------------------------------------------------------------------
##                              BUILD FOR WINDOWS
##---------------------------------------------------------------------------------

ifeq ($(OS),Windows_NT)
    PLATFORM = windows
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        PLATFORM = linux
    endif
    ifeq ($(UNAME_S),Darwin)
        PLATFORM = macos
    endif
endif

ifeq ($(PLATFORM),windows)
  LDFLAGS += -mwindows  # Hide console window
  EXE_EXT = .exe
endif

##---------------------------------------------------------------------------------
##                                 BUILD RULES
##---------------------------------------------------------------------------------

$(BUILD_DIR)/$(build)_%.o:$(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/$(build)_%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/$(build)_%.o:$(BACKENDS_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/$(build)_%.o:$(IMPLOT_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<


all: mkdir $(BUILD_DIR)/$(EXE)
	@echo $(build) build complete

$(BUILD_DIR)/$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

cmake:
    $(CMAKE) -S . -B $(CBUILD_DIR)

mkdir:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)