#pragma once

#include <string>
#include <unordered_map>

// dear imgui:
//  standalone example for GLFW + OpenGL 3, using programmable pipeline
//  If you are new to dear imgui, see examples/README.txt and
//  documentation at the top of imgui.cpp.
//  (GLFW is a cross-platform general purpose library for handling windows,
//  inputs, OpenGL/Vulkan graphics context creation, etc.)
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// About OpenGL function loaders:
//  modern OpenGL doesn't have a standard header file and requires individual
//  function pointers to be loaded manually. 
//  Helper libraries are often used for this purpose!
//  Here we are supporting a few common ones: gl3w, glew, glad.
//  You may use another loader/header of your choice (glext, glLoadGen, etc.),
//  or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"

// Configuration file (edit vktuto_config.h or define VKTUTO_USER_CONFIG to 
// set your own filename)
#ifdef VKTUTO_USER_CONFIG
#include VKTUTO_USER_CONFIG
#endif
#if !defined(VKTUTO_DISABLE_INCLUDE_CONFIG_H) || \
     defined(VKTUTO_INCLUDE_CONFIG_H)
#include "vktuto_config.h"
#endif

// utility
#include "vktuto_utility.h"

namespace vktuto {

inline namespace opengl3 {

class BaseApp
{
 public:
  BaseApp();
  virtual ~BaseApp();

  void Run() {
    MainLoop();
  }

 protected:
  utility::ConsoleApp console;

  virtual void Initialize() = 0 { ChangeOutData(); }
  virtual void Update()     = 0 {}
  virtual void Cleanup()    = 0 {}

  GLFWwindow * GetWindow() { return window_; };

  bool & ShowDemo() noexcept { return show_demo_window_; }

  const ImFont * GetNormalFont() const noexcept { 
    return font_map.at(VkTutoFontFlag::Normal).im_font;
  }
  const ImFont * GetBoldFont() const noexcept {
    return font_map.at(VkTutoFontFlag::Bold).im_font;
  }
  const ImFont * GetThinFont() const noexcept {
    return font_map.at(VkTutoFontFlag::Thin).im_font;
  }

  const int GetCustomProg() const noexcept { return program_; }
  const int GetVAO() const noexcept { return vao_; }
  const int GetVBOPosition() const noexcept { return vbo_position_; }
  const int GetVBOColor() const noexcept { return vbo_color_; }
  const int GetIBO() const noexcept { return ibo_; }
  void BindBuffers() const ;

  std::vector<glm::vec3> & GetVertices() { return vertices; }
  std::vector<glm::vec3> & GetColors() { return colors; }
  std::vector<GLuint>   & GetIndices() { return indices; }

  const int GetFBO() const { return fbo_; }
  const int GetRBODepth() const { return rbo_depth_; }
  const int GetTexture() const { return texture_; }
  int & GetTextrueX() { return texture_x_; }
  int & GetTextrueY() { return texture_y_; }

  float & GetCamX() { return cam_x_; }
  float & GetCamY() { return cam_y_; }
  float & GetCamW() { return cam_width_; }
  float & GetCamH() { return cam_height_; }
  float & GetRotX() { return rotate_x_; }
  float & GetRotY() { return rotate_y_; }

  bool & IsFullScreen() { return full_screen_; }

  virtual void ChangeOutData();

  void ChangeDrawMode(int mode);

 private:
  const struct GLVersion { int major, minor; } gl_version_ /*= { 3, 0 }*/;
  const std::string glsl_version_;
  GLFWwindow *window_;
  bool show_demo_window_ = false;
  glm::vec4 clear_color_ = glm::vec4(VKTUTO_CLEAR_COLOR);
  struct Font {
    std::string file;
    ImFont *im_font;
    float size;
  };
  enum class VkTutoFontFlag { Normal = 0, SolidIcon, BrandIcon, Bold, Thin };
  std::unordered_map<VkTutoFontFlag, Font> font_map = {
    {VkTutoFontFlag::Normal,    {VKTUTO_FONT_NORMAL_PATH,     nullptr, 20.f}},
    {VkTutoFontFlag::SolidIcon, {VKTUTO_FONT_ICON_SOLID_PATH, nullptr, 16.f}},
    {VkTutoFontFlag::BrandIcon, {VKTUTO_FONT_ICON_BRAND_PATH, nullptr, 16.f}},
    {VkTutoFontFlag::Bold,      {VKTUTO_FONT_BOLD_PATH,       nullptr, 22.f}},
    {VkTutoFontFlag::Thin,      {VKTUTO_FONT_THIN_PATH,       nullptr, 18.f}}
  };
  enum class GLDrawMode { Point = GL_POINT,
                          Line = GL_LINE,
                          Fill = GL_FILL } draw_mode = GLDrawMode::Fill;

  GLuint program_;
  GLuint vao_;
  GLuint vbo_position_;
  GLuint vbo_color_;
  GLuint ibo_;

  std::vector<glm::vec3> vertices, colors;
  std::vector<GLuint> indices;

  GLuint fbo_;
  GLuint rbo_depth_;
  GLuint texture_;
  int texture_x_;
  int texture_y_;

  // world coordinates of lower-left corner of the window
  float cam_x_ = -2.0, cam_y_ = -2.0;
  float cam_height_ = 4.0;
  float cam_width_ = cam_height_; //* io.DisplaySize.x / float(io.DisplaySize.y)

  float rotate_x_ = 0.0f, rotate_y_ = 0.0f;

  bool full_screen_ = false;

  static void ErrorCallback(int error, const char *description);
  void InitGLFWWithErrorCallback() const noexcept(false);
  const GLVersion DecideGLVersion(
      int major_version, int minor_version) const noexcept;
  const std::string DecideGLSLVersion(
      const GLVersion gl_version) const noexcept;
  void SetGLVersion() const;

  GLFWwindow * CreateGLFWWindow(
      unsigned int width,
      unsigned int height,
      const std::string &title) noexcept(false);
  void InitGLLoader() const noexcept(false);

  void SetupImGuiContext() const;
  void BindPlatformAndRenderer(
      GLFWwindow *window,
      const std::string &glsl_version);
  
  void SetImguiStyle() const noexcept;
  void LoadFonts();

  void MainLoop();

  void InitCustomGL();
  std::string ReadShaderFile(const char *file_name);
  void CompileShader(const char *file_path, GLuint shader_ID);
  GLuint LoadShaders(const char *vertex_file_path, const char *fragment_file_path);
  void DestroyCustomGL();

  void CustomGLDraw();
}; // class BaseApp

} // inline namespace opengl3

} // namespace vktuto