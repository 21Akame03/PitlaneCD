#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# You will need GLFW (http://www.glfw.org):
# Linux:
#   apt-get install libglfw-dev
# Mac OS X:
#   brew install glfw
# MSYS2:
#   pacman -S --noconfirm --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-glfw
#

#CXX = g++
#CXX = clang++

EXE = example_glfw_opengl3
# Primary dependency locations (expected as sibling checkouts)
IMGUI_DIR ?= ../imgui
IMPLOT_DIR ?= ../implot
IMGUIFILEDIALOG_DIR ?= ../ImGuiFileDialog
VECTOR_DBC_DIR ?= ../vector_dbc
VECTOR_DBC_BUILD_DIR ?= $(VECTOR_DBC_DIR)/build
VECTOR_DBC_LIB ?= $(VECTOR_DBC_BUILD_DIR)/src/Vector/DBC/libVector_DBC.dylib

# Fallback to CMake FetchContent outputs (generated under build/_deps)
IMGUI_FALLBACK := build/_deps/imgui-src
IMPLOT_FALLBACK := build/_deps/implot-src
IMGUIFILEDIALOG_FALLBACK := build/_deps/imguifiledialog-src
VECTOR_DBC_FALLBACK := build/_deps/vector_dbc-src
VECTOR_DBC_FALLBACK_BUILD := build/_deps/vector_dbc-build

# Prefer sibling checkouts, otherwise use fetched sources if present
ifneq ("$(wildcard $(IMGUI_DIR)/imgui.h)","")
else ifneq ("$(wildcard $(IMGUI_FALLBACK)/imgui.h)","")
	IMGUI_DIR := $(IMGUI_FALLBACK)
endif

ifneq ("$(wildcard $(IMPLOT_DIR)/implot.h)","")
else ifneq ("$(wildcard $(IMPLOT_FALLBACK)/implot.h)","")
	IMPLOT_DIR := $(IMPLOT_FALLBACK)
endif

ifneq ("$(wildcard $(IMGUIFILEDIALOG_DIR)/ImGuiFileDialog.h)","")
else ifneq ("$(wildcard $(IMGUIFILEDIALOG_FALLBACK)/ImGuiFileDialog.h)","")
	IMGUIFILEDIALOG_DIR := $(IMGUIFILEDIALOG_FALLBACK)
endif

ifneq ("$(wildcard $(VECTOR_DBC_BUILD_DIR)/src/Vector/DBC/libVector_DBC.*)","")
else ifneq ("$(wildcard $(VECTOR_DBC_FALLBACK_BUILD)/src/Vector/DBC/libVector_DBC.*)","")
	VECTOR_DBC_DIR := $(VECTOR_DBC_FALLBACK)
	VECTOR_DBC_BUILD_DIR := $(VECTOR_DBC_FALLBACK_BUILD)
endif
VECTOR_DBC_LIB := $(firstword $(wildcard $(VECTOR_DBC_BUILD_DIR)/src/Vector/DBC/libVector_DBC.*))
SRC_DIR = src
SOURCES = main.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += $(IMPLOT_DIR)/implot.cpp $(IMPLOT_DIR)/implot_items.cpp $(IMPLOT_DIR)/implot_demo.cpp
SOURCES += $(IMGUIFILEDIALOG_DIR)/ImGuiFileDialog.cpp

# pull in every .cpp under src (any depth)
SOURCES += $(shell find $(SRC_DIR) -name '*.cpp')
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
VPATH = . $(IMGUI_DIR) $(IMGUI_DIR)/backends $(IMPLOT_DIR) $(IMGUIFILEDIALOG_DIR) $(shell find $(SRC_DIR) -type d)
UNAME_S := $(shell uname -s)
LINUX_GL_LIBS = -lGL

CXXFLAGS = -std=c++17 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I$(IMPLOT_DIR) -I$(SRC_DIR) -I$(IMGUIFILEDIALOG_DIR)
# Prefer local vector_dbc headers if present
ifneq ("$(wildcard $(VECTOR_DBC_DIR)/src/Vector/DBC/Network.h)","")
	CXXFLAGS += -I$(VECTOR_DBC_DIR)/src -I$(VECTOR_DBC_BUILD_DIR)/src
endif
CXXFLAGS += -g -Wall -Wformat
LIBS = $(VECTOR_DBC_LIB)

##---------------------------------------------------------------------
## OPENGL ES
##---------------------------------------------------------------------

## This assumes a GL ES library available in the system, e.g. libGLESv2.so
# CXXFLAGS += -DIMGUI_IMPL_OPENGL_ES2
# LINUX_GL_LIBS = -lGLESv2

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += $(LINUX_GL_LIBS) `pkg-config --static --libs glfw3`

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	LIBS += -L/usr/local/lib -L/opt/local/lib -L/opt/homebrew/lib -L$(VECTOR_DBC_BUILD_DIR)/src/Vector/DBC
	LIBS += -Wl,-rpath,$(VECTOR_DBC_BUILD_DIR)/src/Vector/DBC
	#LIBS += -lglfw3
	LIBS += -lglfw

	CXXFLAGS += -I/usr/local/include -I/opt/local/include -I/opt/homebrew/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lglfw3 -lgdi32 -lopengl32 -limm32

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
