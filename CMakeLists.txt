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
FetchContent_Declare (
  nl_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.12.0
)
FetchContent_Declare(
  mdflib
  GIT_REPOSITORY https://github.com/ihedvall/mdflib.git
  GIT_TAG main
)
FetchContent_Declare(
  boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-cmake.tar.gz
  EXCLUDE_FROM_ALL
)
# On macOS (native or cross-compiling from macOS), Homebrew flex headers may
# not be on the default search path.  Flex/bison are host tools needed at
# build time, so we must locate them even when CMAKE_SYSTEM_NAME != Darwin.
if(APPLE OR CMAKE_CROSSCOMPILING)
  # Flex/bison are host tools — bypass cross-compilation sysroot restrictions
  find_program(FLEX_EXECUTABLE flex
    HINTS /opt/homebrew/opt/flex/bin /usr/local/opt/flex/bin
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
  )
  find_path(FLEX_INCLUDE_DIR FlexLexer.h
    HINTS /opt/homebrew/opt/flex/include /usr/local/opt/flex/include
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
  )
  # Propagate into cache so vector_dbc's find_package(FLEX) picks it up
  set(FLEX_INCLUDE_DIRS "${FLEX_INCLUDE_DIR}" CACHE PATH "" FORCE)
elseif(WIN32 AND NOT CMAKE_CROSSCOMPILING)
  # On native Windows, probe common MSYS2 install locations first.
  # This is a no-op if flex/bison are already on PATH (e.g. inside MSYS2 shell).
  foreach(_msys "C:/msys64" "C:/msys2" "$ENV{MSYS_ROOT}")
    if(EXISTS "${_msys}/usr/bin/flex.exe")
      find_program(FLEX_EXECUTABLE flex
        HINTS "${_msys}/usr/bin"
        NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
      find_program(BISON_EXECUTABLE bison
        HINTS "${_msys}/usr/bin"
        NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
      find_path(FLEX_INCLUDE_DIR FlexLexer.h
        HINTS "${_msys}/usr/include"
        NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
      set(FLEX_INCLUDE_DIRS "${FLEX_INCLUDE_DIR}" CACHE PATH "" FORCE)
      break()
    endif()
  endforeach()
  # Fallback: WinFlexBison (choco) or any flex/bison already on system PATH.
  if(NOT FLEX_EXECUTABLE)
    find_program(FLEX_EXECUTABLE  NAMES flex win_flex)
    find_program(BISON_EXECUTABLE NAMES bison win_bison)
  endif()
endif()

# Disable vector_dbc extras we don't need
set(OPTION_RUN_DOXYGEN OFF CACHE BOOL "" FORCE)
set(OPTION_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(OPTION_BUILD_TESTS OFF CACHE BOOL "" FORCE)

# Build Vector_DBC as a static library so we don't need to ship the DLL
# The upstream CMakeLists.txt hardcodes SHARED, so we must patch it before building.
FetchContent_GetProperties(vector_dbc)
if(NOT vector_dbc_POPULATED)
  FetchContent_Populate(vector_dbc)
  # Patch: replace hardcoded SHARED with STATIC so no DLL is produced
  file(READ "${vector_dbc_SOURCE_DIR}/src/Vector/DBC/CMakeLists.txt" _vdbc_cmake)
  string(REPLACE "add_library(\${PROJECT_NAME} SHARED \"\")" "add_library(\${PROJECT_NAME} STATIC \"\")" _vdbc_cmake "${_vdbc_cmake}")
  file(WRITE "${vector_dbc_SOURCE_DIR}/src/Vector/DBC/CMakeLists.txt" "${_vdbc_cmake}")
  add_subdirectory(${vector_dbc_SOURCE_DIR} ${vector_dbc_BINARY_DIR})
endif()

# Only fetch the Boost libraries we actually need
set(BOOST_INCLUDE_LIBRARIES asio system CACHE STRING "" FORCE)

# mdflib build options — library only, no tools/tests/docs
set(MDF_BUILD_TOOL OFF CACHE BOOL "" FORCE)
set(MDF_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(MDF_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(MDF_BUILD_SHARED_LIB OFF CACHE BOOL "" FORCE)

# mdflib needs ZLIB and EXPAT. On macOS these live in the SDK as .tbd stubs
# that CMake's FindZLIB/FindEXPAT don't probe by default — point them at it.
if(APPLE AND NOT CMAKE_CROSSCOMPILING)
  execute_process(COMMAND xcrun --show-sdk-path
    OUTPUT_VARIABLE _macos_sdk OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(_macos_sdk)
    if(NOT ZLIB_LIBRARY)
      set(ZLIB_LIBRARY "${_macos_sdk}/usr/lib/libz.tbd" CACHE FILEPATH "" FORCE)
      set(ZLIB_INCLUDE_DIR "${_macos_sdk}/usr/include" CACHE PATH "" FORCE)
    endif()
    if(NOT EXPAT_LIBRARY)
      set(EXPAT_LIBRARY "${_macos_sdk}/usr/lib/libexpat.tbd" CACHE FILEPATH "" FORCE)
      set(EXPAT_INCLUDE_DIR "${_macos_sdk}/usr/include" CACHE PATH "" FORCE)
    endif()
  endif()
endif()

FetchContent_MakeAvailable(imgui implot ImGuiFileDialog nl_json boost mdflib)

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

# Rename the output binary to PitlaneCD
set_target_properties(glfw_opengl3 PROPERTIES OUTPUT_NAME "PitlaneCD")

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

# Try system GLFW first, fetch from source if not found.
# When cross-compiling, always fetch from source to avoid picking up host libraries.
if(NOT CMAKE_CROSSCOMPILING)
  find_package(glfw3 3.3 QUIET)
endif()
if(NOT glfw3_FOUND)
  message(STATUS "System glfw3 not found — fetching via FetchContent")
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
  )
  FetchContent_MakeAvailable(glfw)
endif()

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
  mdf
  nlohmann_json::nlohmann_json
  Boost::asio
)

if(APPLE)
  target_link_libraries(glfw_opengl3 PRIVATE
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreVideo"
  )
elseif(WIN32)
  target_link_libraries(glfw_opengl3 PRIVATE
    opengl32
    gdi32
    shell32
    ws2_32
    wsock32
  )
  if(MINGW)
    # Statically link libgcc/libstdc++.
    target_link_options(glfw_opengl3 PRIVATE -static-libgcc -static-libstdc++)

    # Statically link libwinpthread.
    # Two-part trick:
    #   1. Link libwinpthread.a directly by path — ld always treats an explicit
    #      .a path as static, regardless of -Bstatic/-Bdynamic state.
    #   2. --as-needed tells ld to skip recording a DLL import if all its symbols
    #      are already satisfied — so the compiler's automatic -lwinpthread (dynamic)
    #      at the end of the link command becomes a no-op.
    target_link_options(glfw_opengl3 PRIVATE -Wl,--as-needed)
    find_library(WINPTHREAD_STATIC libwinpthread.a
      HINTS "C:/msys64/mingw64/lib" "C:/msys2/mingw64/lib"
      NO_DEFAULT_PATH)
    if(WINPTHREAD_STATIC)
      target_link_libraries(glfw_opengl3 PRIVATE "${WINPTHREAD_STATIC}")
      message(STATUS "MinGW: linking winpthread statically from ${WINPTHREAD_STATIC}")
    else()
      message(WARNING "MinGW: libwinpthread.a not found — exe will need the DLL")
    endif()

    # Always locate the DLL — needed for both the post-build copy and installer.
    find_file(WINPTHREAD_DLL libwinpthread-1.dll
      HINTS "C:/msys64/mingw64/bin" "C:/msys2/mingw64/bin"
      NO_DEFAULT_PATH)

    # Copy the DLL next to the exe in the build directory after every build
    # so the app can be run directly without installing.
    if(WINPTHREAD_DLL)
      add_custom_command(TARGET glfw_opengl3 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${WINPTHREAD_DLL}" $<TARGET_FILE_DIR:glfw_opengl3>
        COMMENT "Copying libwinpthread-1.dll to build output dir")
    endif()
  endif()
endif()

# --------------------------------------------------------------------------
# Installer (Windows only, requires NSIS: pacman -S mingw-w64-x86_64-nsis)
# Build with: cmake --build build --target package
# --------------------------------------------------------------------------
if(WIN32)
  install(TARGETS glfw_opengl3
    RUNTIME DESTINATION "."
  )

  # Include winpthread DLL if it couldn't be linked statically
  if(WINPTHREAD_DLL)
    install(FILES "${WINPTHREAD_DLL}" DESTINATION ".")
  endif()

  set(CPACK_GENERATOR "NSIS")
  set(CPACK_PACKAGE_NAME "PitlaneCD")
  set(CPACK_PACKAGE_VENDOR "PitlaneCD")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CAN bus analyser and data logger")
  set(CPACK_PACKAGE_VERSION "1.0.0")
  set(CPACK_PACKAGE_VERSION_MAJOR "1")
  set(CPACK_PACKAGE_VERSION_MINOR "0")
  set(CPACK_PACKAGE_VERSION_PATCH "0")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "PitlaneCD")
  set(CPACK_PACKAGE_EXECUTABLES "PitlaneCD" "PitlaneCD")

  # NSIS-specific
  set(CPACK_NSIS_DISPLAY_NAME "PitlaneCD")
  set(CPACK_NSIS_PACKAGE_NAME "PitlaneCD")
  set(CPACK_NSIS_MUI_FINISHPAGE_RUN "PitlaneCD.exe")
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  # Start Menu shortcut
  set(CPACK_NSIS_CREATE_ICONS_EXTRA
    "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\PitlaneCD.lnk' '$INSTDIR\\\\PitlaneCD.exe'")
  set(CPACK_NSIS_DELETE_ICONS_EXTRA
    "Delete '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\PitlaneCD.lnk'")

  include(CPack)
endif()
