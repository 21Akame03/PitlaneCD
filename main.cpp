 //     ____  ____________    ___    _   ________
 //    / __ \/  _/_  __/ /   /   |  / | / / ____/
 //   / /_/ // /  / / / /   / /| | /  |/ / __/
 //  / ____// /  / / / /___/ ___ |/ /|  / /___
 // /_/   /___/ /_/ /_____/_/  |_/_/ |_/_____/
 //
 //
 // DO NOT OVERWRITE THIS FILE!!
 // THE APP.CPP (MAIN FOR US) IS FOUND UNDER SRC
 //
// Dear ImGui: standalone example application for GLFW + OpenGL 3, using
// programmable pipeline (GLFW is a cross-platform general purpose library for
// handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation,
// etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/
// folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include "app.hpp"
#include "implot.h"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to
// maximize ease of testing and compatibility with old VS compilers. To link
// with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own project
// should not be affected, as you are likely to link with a newer binary of GLFW
// that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) &&                                 \
    !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See
// 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../imgui/examples/libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int, char **) {
  fprintf(stderr, "[DBG] main() entered\n"); fflush(stderr);
  glfwSetErrorCallback(glfw_error_callback);
  fprintf(stderr, "[DBG] calling glfwInit\n"); fflush(stderr);
  if (!glfwInit())
    return 1;
  fprintf(stderr, "[DBG] glfwInit OK\n"); fflush(stderr);

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100 (WebGL 1.0)
  const char *glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
  // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
  const char *glsl_version = "#version 300 es";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  // GL 3.2 + GLSL 150
  const char *glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#else
  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
  // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
#endif

  // Create window with graphics context
  fprintf(stderr, "[DBG] getting primary monitor\n"); fflush(stderr);
  GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
  fprintf(stderr, "[DBG] primary_monitor=%p\n", (void*)primary_monitor); fflush(stderr);
  float main_scale = primary_monitor
      ? ImGui_ImplGlfw_GetContentScaleForMonitor(primary_monitor)
      : 1.0f;
  fprintf(stderr, "[DBG] main_scale=%.2f, creating window\n", main_scale); fflush(stderr);
  GLFWwindow *window =
      glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale),
                       "Pitlane", nullptr, nullptr);
  fprintf(stderr, "[DBG] window=%p\n", (void*)window); fflush(stderr);
  if (window == nullptr)
    return 1;
  glfwMakeContextCurrent(window);
  fprintf(stderr, "[DBG] context current\n"); fflush(stderr);
  glfwSwapInterval(1); // Enable vsync

  // Setup Dear ImGui context
  fprintf(stderr, "[DBG] ImGui CHECKVERSION\n"); fflush(stderr);
  IMGUI_CHECKVERSION();
  fprintf(stderr, "[DBG] ImGui CreateContext\n"); fflush(stderr);
  ImGui::CreateContext();
  fprintf(stderr, "[DBG] ImPlot CreateContext\n"); fflush(stderr);
  ImPlot::CreateContext();
  fprintf(stderr, "[DBG] contexts created\n"); fflush(stderr);
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad;              // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
  // ViewportsEnable creates extra GL contexts per detached window — crashes on
  // many Windows/driver combos. Docking still works without it.
  // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  // io.ConfigViewportsNoAutoMerge = true;
  // io.ConfigViewportsNoTaskBarIcon = true;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup scaling
  ImGuiStyle &style = ImGui::GetStyle();
  style.ScaleAllSizes(
      main_scale); // Bake a fixed style scale. (until we have a solution for
                   // dynamic style scaling, changing this requires resetting
                   // Style + calling this again)
  style.FontScaleDpi =
      main_scale; // Set initial font scale. (using io.ConfigDpiScaleFonts=true
                  // makes this unnecessary. We leave both here for
                  // documentation purpose)
#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 3
  io.ConfigDpiScaleFonts =
      true; // [Experimental] Automatically overwrite style.FontScaleDpi in
            // Begin() when Monitor DPI changes. This will scale fonts but _NOT_
            // scale sizes/padding for now.
  io.ConfigDpiScaleViewports =
      true; // [Experimental] Scale Dear ImGui and Platform Windows when Monitor
            // DPI changes.
#endif


  // Setup Platform/Renderer backends
  fprintf(stderr, "[DBG] ImGui_ImplGlfw_InitForOpenGL\n"); fflush(stderr);
  ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
  ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
  fprintf(stderr, "[DBG] ImGui_ImplOpenGL3_Init\n"); fflush(stderr);
  ImGui_ImplOpenGL3_Init(glsl_version);
  fprintf(stderr, "[DBG] init complete, entering loop\n"); fflush(stderr);

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Construct App here (not as a static local inside the loop) to avoid
  // MinGW's pthread-backed once-guard which can crash on first use.
  fprintf(stderr, "[DBG] constructing App\n"); fflush(stderr);
  App app;
  fprintf(stderr, "[DBG] App ready, entering loop\n"); fflush(stderr);

  // Main loop
#ifdef __EMSCRIPTEN__
  // For an Emscripten build we are disabling file-system access, so let's not
  // attempt to do a fopen() of the imgui.ini file. You may manually call
  // LoadIniSettingsFromMemory() to load settings from your own storage.
  io.IniFilename = nullptr;
  EMSCRIPTEN_MAINLOOP_BEGIN
#else
  while (!glfwWindowShouldClose(window))
#endif
  {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
    // tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
    // your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
    // data to your main application, or clear/overwrite your copy of the
    // keyboard data. Generally you may always pass all inputs to dear imgui,
    // and hide them from your application based on those two flags.
    fprintf(stderr, "[DBG] loop: glfwPollEvents\n"); fflush(stderr);
    glfwPollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }

    // Start the Dear ImGui frame
    fprintf(stderr, "[DBG] loop: OpenGL3_NewFrame\n"); fflush(stderr);
    ImGui_ImplOpenGL3_NewFrame();
    fprintf(stderr, "[DBG] loop: Glfw_NewFrame\n"); fflush(stderr);
    ImGui_ImplGlfw_NewFrame();
    fprintf(stderr, "[DBG] loop: ImGui::NewFrame\n"); fflush(stderr);
    ImGui::NewFrame();

    fprintf(stderr, "[DBG] loop: App::RenderUI\n"); fflush(stderr);
    app.RenderUI();
    fprintf(stderr, "[DBG] loop: ImGui::Render\n"); fflush(stderr);

    // Rendering
    ImGui::Render();
    fprintf(stderr, "[DBG] loop: glViewport\n"); fflush(stderr);
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    fprintf(stderr, "[DBG] loop: RenderDrawData\n"); fflush(stderr);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we
    // save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call
    //  glfwMakeContextCurrent(window) directly)
    fprintf(stderr, "[DBG] loop: UpdatePlatformWindows\n"); fflush(stderr);
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      GLFWwindow *backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      fprintf(stderr, "[DBG] loop: RenderPlatformWindowsDefault\n"); fflush(stderr);
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }

    fprintf(stderr, "[DBG] loop: SwapBuffers\n"); fflush(stderr);
    glfwSwapBuffers(window);
  }
#ifdef __EMSCRIPTEN__
  EMSCRIPTEN_MAINLOOP_END;
#endif

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
