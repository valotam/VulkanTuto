#include <iostream>
#include <stdexcept>
#include <fstream>
#include <algorithm>
#include <string>

#include "opengl3_base.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

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
  InitCustomGL();

  SetImguiStyle();
  LoadFonts();

  Initialize();
}

BaseApp::~BaseApp() {
  Cleanup();

  // Cleanup
  DestroyCustomGL();

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
    switch (draw_mode)
    {
    case vktuto::opengl3::BaseApp::GLDrawMode::Point:
      glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
      break;
    case vktuto::opengl3::BaseApp::GLDrawMode::Line:
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      break;
    case vktuto::opengl3::BaseApp::GLDrawMode::Fill: default:
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      break;
    }
    glViewport(0, 0, display_w, display_h);
    glClearColor(
      clear_color_.x,
      clear_color_.y,
      clear_color_.z,
      clear_color_.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    CustomGLDraw();

    glfwMakeContextCurrent(window_);
    glfwSwapBuffers(window_);
  }

}

void BaseApp::InitCustomGL() {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  ////////////////////////////////////////////////////////////////////
  // Create program for drawing cube, create VBOs and copy the data //
  ////////////////////////////////////////////////////////////////////
  program_ = LoadShaders("vertex_shader.vs", "fragment_shader.fs");

  /////////////////////////
  // Create and bind VBO //
  /////////////////////////
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_position_);
  glGenBuffers(1, &vbo_color_);
  glGenBuffers(1, &ibo_);

  BindBuffers();

  glGenVertexArrays(1, &vao2_);
  glGenBuffers(1, &vbo_position2_);
  glGenBuffers(1, &vbo_color2_);
  glGenBuffers(1, &ibo2_);

  BindBuffers2();

  ////////////////////////////////////////////////////////////////////////
  // Create and bind framebuffer, attach a depth buffer to it           //
  // Create the texture to render to, and attach it to the framebuffer  //
  ////////////////////////////////////////////////////////////////////////
  ImGuiIO &io = ImGui::GetIO();

  glGenFramebuffers(1, &fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

  glGenRenderbuffers(1, &rbo_depth_);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth_);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, io.DisplaySize.x, io.DisplaySize.y);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth_);

  glGenTextures(1, &texture_);
  glBindTexture(GL_TEXTURE_2D, texture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, io.DisplaySize.x, io.DisplaySize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_, 0);
  GLenum draw_options[1] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers(1, draw_options);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "Error in setting up the framebuffer" << std::endl;
  }
}

void BaseApp::BindBuffers() const {
  glBindVertexArray(vao_);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_position_);
  glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(glm::vec3), vertices_.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_color_);
  glBufferData(GL_ARRAY_BUFFER, colors_.size() * sizeof(glm::vec3), colors_.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(GLuint), indices_.data(), GL_STATIC_DRAW);
  glBindVertexArray(0);
}

void BaseApp::BindBuffers2() const {
  glBindVertexArray(vao2_);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_position2_);
  glBufferData(GL_ARRAY_BUFFER, vertices2_.size() * sizeof(glm::vec3), vertices2_.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_color2_);
  glBufferData(GL_ARRAY_BUFFER, colors2_.size() * sizeof(glm::vec3), colors2_.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo2_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices2_.size() * sizeof(GLuint), indices2_.data(), GL_STATIC_DRAW);
  glBindVertexArray(0);
}

std::string BaseApp::ReadShaderFile(const char *file_name) {
  /////////////////////////////////////////////////////////////
  // Read content of "filename" and return it as a c-string. //
  /////////////////////////////////////////////////////////////
  std::string str;
  std::cout << "Reading " << file_name << std::endl;
  std::ifstream in(file_name, std::ios::ate);
  if (in.is_open()) {
    in.seekg(0, in.end);
    long fsize = in.tellg();
    in.seekg(0, in.beg);
    std::cout << "Filesize = " << fsize << std::endl;
    str.resize(fsize + 1);
    in.read(str.data(), fsize);
    str.data()[fsize] = '\0';
    in.close();
  }
  else std::cout << "Unable to open file" << std::endl;

  return str;
}

void BaseApp::CompileShader(const char *file_path, GLuint shader_ID) {
  GLint Result = GL_FALSE;
  int InfoLogLength;

  std::string shaderString = ReadShaderFile(file_path);
  const GLchar *ShaderCode = shaderString.c_str();

  // Compile Shader
  printf("Compiling shader : %s\n", file_path);
  glShaderSource(shader_ID, 1, &ShaderCode, NULL);
  glCompileShader(shader_ID);

  // Check Shader
  glGetShaderiv(shader_ID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(shader_ID, GL_INFO_LOG_LENGTH, &InfoLogLength);

  if (Result == GL_FALSE) {
    char ShaderErrorMessage[9999];
    glGetShaderInfoLog(shader_ID, InfoLogLength, NULL, ShaderErrorMessage);
    printf("%s", ShaderErrorMessage);
  }
}

GLuint BaseApp::LoadShaders(const char *vertex_file_path, const char *fragment_file_path) {
  printf("Creating shaders\n");
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  CompileShader(vertex_file_path, VertexShaderID);
  CompileShader(fragment_file_path, FragmentShaderID);


  printf("Create and linking program\n");
  GLuint program = glCreateProgram();
  glAttachShader(program, VertexShaderID);
  glAttachShader(program, FragmentShaderID);
  glLinkProgram(program);

  // Check the program
  GLint Result = GL_FALSE;
  int InfoLogLength;

  glGetProgramiv(program, GL_LINK_STATUS, &Result);
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &InfoLogLength);

  if (InfoLogLength > 0) {
    GLchar ProgramErrorMessage[9999];
    glGetProgramInfoLog(program, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    printf("%s\n", &ProgramErrorMessage[0]); fflush(stdout);
  }

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return program;
}

void BaseApp::DestroyCustomGL() {
  // optional: de-allocate all resources once they've outlived their purpose:
  // ------------------------------------------------------------------------
  glDeleteVertexArrays(1, &vao_);
  glDeleteBuffers(1, &vbo_position_);
  glDeleteBuffers(1, &vbo_color_);
  glDeleteBuffers(1, &ibo_);

  glDeleteVertexArrays(1, &vao2_);
  glDeleteBuffers(1, &vbo_position2_);
  glDeleteBuffers(1, &vbo_color2_);
  glDeleteBuffers(1, &ibo2_);

  glDeleteFramebuffers(1, &fbo_);
  glDeleteRenderbuffers(1, &rbo_depth_);
  glDeleteTextures(1, &texture_);
}

void BaseApp::CustomGLDraw() {
  /*
  const GLfloat instanceStart[] = {
      0.7, 0.0, 0.0, 0.0,
      0.0, 0.5, 0.0, 0.0,
      0.0, 0.2, 0.4, 0.6
  };

  const GLfloat instanceStop[] = {
      1.0, 0.0, 0.0, 0.0,
      0.3, 1.0, 0.0, 0.0,
      0.1, 0.3, 0.5, 0.8
  };

  const GLfloat instanceIndex[] {
      0.0, -1.0, -1.0, -1.0,
      1.0, 0.0, -1.0, -1.0,
      0.0, 2.0, 3.0, 4.0
  };

  glBindBuffer(GL_ARRAY_BUFFER, startbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(instanceStart), 0, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(instanceStart), instanceStart);

  glBindBuffer(GL_ARRAY_BUFFER, stopbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(instanceStop), 0, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(instanceStop), instanceStop);

  glBindBuffer(GL_ARRAY_BUFFER, indexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(instanceIndex), 0, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(instanceIndex), instanceIndex);
  */

  //////////////////////////////////////////////////////////////////////
  // Draw a cube to a framebuffer texture using orthogonal projection //
  // with camera looking along the negative Z axis                    //
  // The cube rotates around a random axis                            //
  //////////////////////////////////////////////////////////////////////

  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  glViewport(0, 0, texture_x_, texture_y_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(program_);

  /////////////////////////////////////////////////////////////////
  // View matrix is identity matrix for 2D,                      //
  // camera movement put in orthogonal projection matrix instead //
  /////////////////////////////////////////////////////////////////
  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection = glm::ortho<float>(cam_x_, cam_x_ + cam_width_,
                                           cam_y_, cam_y_ + cam_height_, -10, 10);
  //mat4 view = view(vec3(1,0,0), vec3(0,1,0), vec3(0,0,-1), vec3(cam_x,cam_y,6));
  //mat4 projection = glm::perspective(60, resx/float(resy), 0.01, 10.0);

  ////////////////////////////////
  // same RNG seed every frame  //
  // "minimal standard" LCG RNG //
  ////////////////////////////////
  glm::mat4 model = glm::rotate(glm::mat4(1.0f), rotate_x_, glm::vec3(1.0, 0.0, 0.0))
                    *glm::rotate(glm::mat4(1.0f), rotate_y_, glm::vec3(0.0, 1.0, 0.0)); 

  glm::mat4 MVP = projection * view * model;
  glUniformMatrix4fv(glGetUniformLocation(program_, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

  // Draw!
  glBindVertexArray(vao_);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_position_);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_color_);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  glVertexAttribDivisor(0, 0);
  glVertexAttribDivisor(1, 0);
  glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, NULL);
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glBindVertexArray(0);

  glBindVertexArray(vao2_);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_position2_);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_color2_);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  glVertexAttribDivisor(0, 0);
  glVertexAttribDivisor(1, 0);
  glDrawElements(GL_LINES, indices2_.size(), GL_UNSIGNED_INT, NULL);
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glBindVertexArray(0);

  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BaseApp::ChangeOutData() {
  vertices_ = {
    glm::vec3(-1.0f, -1.0f,  1.0f),
    glm::vec3( 1.0f, -1.0f,  1.0f),
    glm::vec3(-1.0f,  1.0f,  1.0f),
    glm::vec3( 1.0f,  1.0f,  1.0f),
    glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3( 1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f,  1.0f, -1.0f),
    glm::vec3( 1.0f,  1.0f, -1.0f)
  };

  colors_ = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(1.0f,  0.0f,  0.0f),
    glm::vec3(1.0f,  1.0f,  0.0f),
    glm::vec3(1.0f,  1.0f,  1.0f),
    glm::vec3(0.0f,  1.0f,  1.0f),
    glm::vec3(0.0f,  0.0f,  1.0f),
    glm::vec3(0.0f,  1.0f,  0.0f),
    glm::vec3(1.0f,  0.0f,  1.0f)
  };

  indices_ = {
      0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1
  };

  BindBuffers();
}

void BaseApp::ChangeOutData2() {
  vertices2_ = {
    glm::vec3(-1.0f, -1.0f,  1.0f),
    glm::vec3(1.0f, -1.0f,  1.0f),
    glm::vec3(-1.0f,  1.0f,  1.0f),
  };

  colors2_ = {
    glm::vec3(1.0f,  1.0f,  1.0f),
    glm::vec3(1.0f,  1.0f,  1.0f),
    glm::vec3(1.0f,  1.0f,  1.0f),
  };

  indices2_ = {
      0, 1, 0, 2
  };

  BindBuffers2();
}

void BaseApp::ChangeDrawMode(int mode) {
  switch (mode)
  {
  case 0:
    draw_mode = GLDrawMode::Point;
    break;
  case 1:
    draw_mode = GLDrawMode::Line;
    break;
  case 2: default:
    draw_mode = GLDrawMode::Fill;
    break;
  }
}

} // inline namespace opengl3

} // namespace vktuto