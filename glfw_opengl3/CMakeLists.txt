cmake_minimum_required(VERSION 3.15)
project(glfw_opengl3_example LANGUAGES CXX)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../imgui)
set(IMPLOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../implot)
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

add_executable(glfw_opengl3
  main.cpp
  ${SRC_SOURCES}
  ${IMGUI_SOURCES}
  ${IMPLOT_SOURCES}
)

target_include_directories(glfw_opengl3 PRIVATE
  ${IMGUI_DIR}
  ${IMGUI_DIR}/backends
  ${IMPLOT_DIR}
  ${SRC_DIR}
)

target_compile_features(glfw_opengl3 PRIVATE cxx_std_11)

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
)

if(APPLE)
  target_link_libraries(glfw_opengl3 PRIVATE
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreVideo"
  )
endif()
