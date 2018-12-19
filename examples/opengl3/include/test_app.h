#pragma once
#include "opengl3_base.h"

namespace vktuto {

inline namespace opengl3 {

class TestApp : public BaseApp {
 public:
  TestApp();
  virtual ~TestApp();

 protected:
  virtual void Initialize() override;
  virtual void Update() override;
  virtual void Cleanup() override;

 private:
  ImGuiWindowFlags window_flags_ = 0;
  bool show_main_window_ = true;

  int resx = 1280;
  int resy = 720;
  float  prevx = -1, prevy = -1; // to track how much the mouse moved between frames
  float cam_x = -2.0, cam_y = -2.0; // world coordinates of lower-left corner of the window
  float cam_height = 4.0;
  float cam_width = cam_height * resx / float(resy);


  // buffer object stuffs
  GLuint cube_program;
  GLuint VertexArrayID;

  GLuint vertexbuffer;
  GLuint colorbuffer;
  GLuint elementbuffer;
  int num_indices;

  // Framebuffer stuff
  GLuint framebuffer;
  GLuint texture;
  GLuint depthbuffer;


  int textureX = resx;
  int textureY = resy;

  float rotateX = 0.0f, rotateY = 0.0f;

  std::string readFile(const char *filename);
  void CompileShader(const char *file_path, GLuint ShaderID);
  GLuint LoadShaders(const char *vertex_file_path, const char *fragment_file_path);

  void init_GL();

  void draw_cube();
  void cube_GUI();

  bool fullscreen = false;

  ImGuiWindowFlags & SetupWindowFlags() const noexcept;

  void ShowMainWindow();
  
  void MenuTabs();
  void SplitView();
  
  void ControlsColumn();
  
  void TabsColumn();
  void CanvasTab();
  void ConsolTab();
}; // class TestApp

} // inline namespace opengl3

} // namespace vktuto