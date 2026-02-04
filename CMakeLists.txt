cmake_minimum_required(VERSION 3.15)
project(glfw_opengl3_example LANGUAGES CXX)


include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        703bd9caab50b139428cea1aaff9974ebee5742e # release-1.10.0
)


FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG docking
)
FetchContent_Declare(
  implot
  GIT_REPOSITORY https://github.com/epezent/implot.git
  GIT_TAG master
)
FetchContent_Declare(
  ImGuiFileDialog
  GIT_REPOSITORY https://github.com/aiekick/ImGuiFileDialog.git
  GIT_TAG master
)
FetchContent_Declare(
  vector_dbc
  GIT_REPOSITORY https://bitbucket.org/tobylorenz/vector_dbc.git
  GIT_TAG master
)
FetchContent_MakeAvailable(imgui implot ImGuiFileDialog vector_dbc)

# Ensure vector_dbc finds its own headers when used as a subproject
if(TARGET Vector_DBC)
  target_include_directories(Vector_DBC PUBLIC
    ${vector_dbc_SOURCE_DIR}/src
    ${vector_dbc_BINARY_DIR}/src)
endif()

# Point ImGuiFileDialog at the fetched imgui headers when the package lookup fails
if(TARGET ImGuiFileDialog)
  target_include_directories(ImGuiFileDialog PUBLIC ${imgui_SOURCE_DIR})
endif()

set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMPLOT_DIR ${implot_SOURCE_DIR})
set(IMGUIFILEDIALOG_DIR ${imguifiledialog_SOURCE_DIR})
set(VECTOR_DBC_BINARY_DIR ${vector_dbc_BINARY_DIR})
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)


file(GLOB_RECURSE SRC_SOURCES CONFIGURE_DEPENDS
  ${SRC_DIR}/*.cpp
  ${SRC_DIR}/*.hpp
)

set(IMGUI_SOURCES
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_demo.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

file(GLOB IMPLOT_SOURCES CONFIGURE_DEPENDS
  ${IMPLOT_DIR}/*.cpp
)

file(GLOB IMGUIFILEDIALOG_SOURCES CONFIGURE_DEPENDS
  ${IMGUIFILEDIALOG_DIR}/*.cpp
)


add_executable(glfw_opengl3
  main.cpp
  ${SRC_SOURCES}
  ${IMGUI_SOURCES}
  ${IMPLOT_SOURCES}
  ${IMGUIFILEDIALOG_SOURCES}
)

target_include_directories(glfw_opengl3 PRIVATE
  ${IMGUI_DIR}
  ${IMGUI_DIR}/backends
  ${IMPLOT_DIR}
  ${IMGUIFILEDIALOG_DIR}
  ${SRC_DIR}
  ${VECTOR_DBC_BINARY_DIR}/src/Vector/DBC
)

target_compile_features(glfw_opengl3 PRIVATE cxx_std_17)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

if(TARGET glfw)
  set(GLFW_TARGET glfw)
elseif(TARGET glfw3)
  set(GLFW_TARGET glfw3)
else()
  message(FATAL_ERROR "glfw3 package found but no glfw/glfw3 target was exported")
endif()

target_link_libraries(glfw_opengl3 PRIVATE
  OpenGL::GL
  ${GLFW_TARGET}
  Vector_DBC
)

if(APPLE)
  target_link_libraries(glfw_opengl3 PRIVATE
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreVideo"
  )
endif()
