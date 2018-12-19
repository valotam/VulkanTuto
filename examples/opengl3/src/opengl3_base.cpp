#include <iostream>
#include <stdexcept>

#include "opengl3_base.h"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to
//          maximize ease of testing and compatibility with old VS compilers.
//          To link with VS2010-era libraries, VS2015+ requires linking with 
//          legacy_stdio_definitions.lib, which we do using this pragma. 
//          Your own project should not be affected, as you are likely to link
//          with a newer binary of GLFW that is adequate for your version of
//          Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

namespace vktuto {

inline namespace opengl3 {

BaseApp::BaseApp() : 
    gl_version_(DecideGLVersion(
                    VKTUTO_OPENGL_MAJOR_VERSION,
                    VKTUTO_OPENGL_MINOR_VERSION)),
    glsl_version_(DecideGLSLVersion(gl_version_)) {
  InitGLFWWithErrorCallback();
  SetGLVersion();

  window_ = CreateGLFWWindow(
      VKTUTO_WINDOW_WIDTH,
      VKTUTO_WINDOW_HEIGHT,
      VKTUTO_WINDOW_TITLE);
  InitGLLoader();

  SetupImGuiContext();
  BindPlatformAndRenderer(window_, glsl_version_);

  SetImguiStyle();
  LoadFonts();

  Initialize();
}

BaseApp::~BaseApp() {
  Cleanup();

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window_);
  glfwTerminate();
}

void BaseApp::ErrorCallback(int error, const char *description) {
  std::cerr << "Glfw Error " << error << ": " << description << std::endl;
}

void BaseApp::InitGLFWWithErrorCallback() const noexcept(false) {
  // Setup window
  glfwSetErrorCallback(ErrorCallback);
  if (!glfwInit()) throw std::runtime_error("GLFW initialization failed!");
}

const BaseApp::GLVersion BaseApp::DecideGLVersion(
    int major_version,
    int minor_version) const noexcept {
  return { major_version, minor_version };
}

const std::string BaseApp::DecideGLSLVersion(
    const GLVersion gl_version) const noexcept {
  std::string glsl_version("#version ");
  if (gl_version.major == 2 && gl_version.minor == 0) {
    glsl_version.append("110");
  } else if (gl_version.major == 2 && gl_version.minor == 1) {
    glsl_version.append("120");
  } else if (gl_version.major == 3 && gl_version.minor == 0) {
    glsl_version.append("130");
  } else if (gl_version.major == 3 && gl_version.minor == 1) {
    glsl_version.append("140");
  } else if (gl_version.major == 3 && gl_version.minor == 2) {
    glsl_version.append("150");
  } else if (gl_version.major == 3 && gl_version.minor == 3) {
    glsl_version.append("330");
  } else if (gl_version.major == 4 && gl_version.minor == 0) {
    glsl_version.append("400");
  } else if (gl_version.major == 4 && gl_version.minor == 1) {
    glsl_version.append("410 core");
  } else if (gl_version.major == 4 && gl_version.minor == 2) {
    glsl_version.append("420 core");
  } else if (gl_version.major == 4 && gl_version.minor == 3) {
    glsl_version.append("430 core");
  }
  return glsl_version.c_str();
}

void BaseApp::SetGLVersion() const {
  // Decide GL+GLSL versions
#if __APPLE__
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_version_.major);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_version_.minor);
  if ((gl_version_.major == 3 && gl_version_.minor >= 2) ||
      (gl_version_.major >= 4)) {
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);// 3.2+ only
  }
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);      // Required on Mac
#else
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_version_.major);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_version_.minor);
  if (gl_version_.major >= 3) {
    if ((gl_version_.minor >= 2) || (gl_version_.major >= 4))
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);// 3.2+
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+
  }
#endif
}

GLFWwindow * BaseApp::CreateGLFWWindow(
    unsigned int width,
    unsigned int height,
    const std::string &title) noexcept(false) {
  // Create window with graphics context
  GLFWwindow *window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
  if (window == NULL) throw std::runtime_error("GLFW creatation failed!");
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  return window;
}

void BaseApp::InitGLLoader() const noexcept(false) {
  // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
  bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
  bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
  bool err = gladLoadGL() == 0;
#else
  bool err = false;
  // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM,
  // your loader is likely to requires some form of initialization.
#endif
  if (err)
  {
    std::cerr << "Failed to initialize OpenGL loader!" << std::endl;
    throw std::runtime_error("OpenGL loader initialization failed!");
  }
}

void BaseApp::SetupImGuiContext() const {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  //ImGuiIO &io = ImGui::GetIO(); (void)io;
  //// Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  //// Enable Gamepad Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
}

void BaseApp::BindPlatformAndRenderer(
    GLFWwindow *window,
    const std::string &glsl_version) {
  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version.c_str());
}

void BaseApp::SetImguiStyle() const noexcept {
  // Setup Style
  ImGui::StyleColorsDark();
}

void BaseApp::LoadFonts() {
  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font.
  //    You can also load multiple fonts and use ImGui::PushFont()/PopFont()
  //    to select them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if 
  //    you need to select the font among multiple.
  // - If the file cannot be loaded, the function will return NULL.
  //    Please handle those errors in your application
  //    (e.g. use an assertion, or display an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and
  //    stored into a texture when calling
  //    ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame
  //    below will call.
  // - Read 'misc/fonts/README.txt' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string
  //    literal you need to write a double backslash \\ !
  ImGuiIO &io = ImGui::GetIO();
  ImFontAtlas *atlas = io.Fonts;
  ImFontConfig icons_config;
               icons_config.MergeMode = true;
               icons_config.PixelSnapH = true;
  {
    Font &font = font_map[VkTutoFontFlag::Normal];
    font.im_font = atlas->AddFontFromFileTTF(font.file.c_str(), font.size,
                                             NULL,
                                             atlas->GetGlyphRangesKorean());
    IM_ASSERT(font.im_font != NULL);
  }
  {
    // merge in icons from Font Awesome
    const ImWchar icons_ranges_solid[] = { ICON_MIN_FAS, ICON_MAX_FAS, 0 };
    Font &font = font_map[VkTutoFontFlag::SolidIcon];
    font.im_font = atlas->AddFontFromFileTTF(font.file.c_str(), font.size,
                                             &icons_config,
                                             icons_ranges_solid);
    IM_ASSERT(font.im_font != NULL);
    // use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid
  }
  {
    const ImWchar icons_ranges_brand[] = { ICON_MIN_FAB, ICON_MAX_FAB, 0 };
    Font &font = font_map[VkTutoFontFlag::BrandIcon];
    font.im_font = atlas->AddFontFromFileTTF(font.file.c_str(), font.size,
                                             &icons_config,
                                             icons_ranges_brand);
    IM_ASSERT(font.im_font != NULL);
  }
  {
    Font &font = font_map[VkTutoFontFlag::Bold];
    font.im_font = atlas->AddFontFromFileTTF(font.file.c_str(), font.size,
                                             NULL,
                                             atlas->GetGlyphRangesKorean());
    IM_ASSERT(font.im_font != NULL);
  }
  {
    Font &font = font_map[VkTutoFontFlag::Thin];
    font.im_font = atlas->AddFontFromFileTTF(font.file.c_str(), font.size,
                                             NULL,
                                             atlas->GetGlyphRangesKorean());
    IM_ASSERT(font.im_font != NULL);
  }

  io.Fonts->Build();
}

void BaseApp::MainLoop() {
  // Main loop
  while (!glfwWindowShouldClose(window_)) {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags
    //  to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
    //    your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
    //    data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them
    //    from your application based on those two flags.
    glfwPollEvents();
    glfwGetWindowSize(window_, &window_size_.width, &window_size_.height);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    Update();

    // 1. Show the big demo window (Most of the sample code is in
    //    ImGui::ShowDemoWindow()! You can browse its code to learn more about
    //    Dear ImGui!).
    if (show_demo_window_)
      ImGui::ShowDemoWindow(&show_demo_window_);

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwMakeContextCurrent(window_);
    glfwGetFramebufferSize(window_, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(
      clear_color_.x,
      clear_color_.y,
      clear_color_.z,
      clear_color_.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwMakeContextCurrent(window_);
    glfwSwapBuffers(window_);
  }

}

} // inline namespace opengl3

} // namespace vktuto