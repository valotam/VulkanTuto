#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>

#include "test_app.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace vktuto {

inline namespace opengl3 {

TestApp::TestApp() {
  Initialize();
}

TestApp::~TestApp() {
  Cleanup();
}

void TestApp::Initialize() {
  init_GL();
  window_flags_ = SetupWindowFlags();
}

void TestApp::Update() {
  if (show_main_window_) ShowMainWindow();
}

void TestApp::Cleanup() {
}

ImGuiWindowFlags & TestApp::SetupWindowFlags() const noexcept
{
  // Demonstrate the various window flags.
  // Typically you would just use the default!
  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  //window_flags |= ImGuiWindowFlags_NoScrollbar;
  window_flags |= ImGuiWindowFlags_MenuBar;
  window_flags |= ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoResize;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoNav;
  //window_flags |= ImGuiWindowFlags_NoBackground;
  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
  return window_flags;
}

void TestApp::ShowMainWindow() {
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(this->GetWindowWidth(),
                                  this->GetWindowHeight()));
  // Main body of the Demo window starts here.
  if (!ImGui::Begin("##control-tab-split-window", NULL, window_flags_)) {
    // Early out if the window is collapsed, as an optimization.
    ImGui::End();
    return;
  }

  // Menu
  MenuTabs();

  // Split view: control-tab-split-view
  SplitView();

  // End of ShowMainWindow()
  ImGui::End();
}

void TestApp::MenuTabs() {
  // Menu
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Menu")) {
      ImGui::MenuItem(ICON_FA_FILE_IMPORT " Load files", "Ctrl+L");
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Examples")) {
      ImGui::MenuItem("Demonstration", "Ctrl+D", &ShowDemo());
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
      ImGui::MenuItem("About this program", NULL);
      ImGui::MenuItem("Github " ICON_FA_GITHUB, NULL);
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
}

void TestApp::SplitView() {
  // Split view
  ImGui::BeginGroup();
  ImGui::Columns(2);
  ImGui::SetColumnOffset(0, 0.f);
  ImGui::SetColumnOffset(1, ImGui::GetWindowWidth() * 0.25f);
  
  bool child_border = true;
  ImVec2 win_size = ImVec2(0.f, ImGui::GetContentRegionAvail().y);

  ImGui::BeginChild("##col-1-controls", win_size, child_border);
  ControlsColumn();
  ImGui::EndChild();

  ImGui::NextColumn();
  ImGui::BeginChild("##col-2-tabs", win_size, child_border);
  TabsColumn();
  ImGui::EndChild();

  ImGui::EndGroup();
  ImGui::Columns(1);
}

void TestApp::ControlsColumn() {
}

void TestApp::TabsColumn() {
  if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None)) {
    CanvasTab();
    ConsolTab();
    ImGui::EndTabBar();
  }
}

void TestApp::CanvasTab() {
  if (ImGui::BeginTabItem("Canvas")) {
    cube_GUI();
    ImGui::EndTabItem();
  }
}

void TestApp::ConsolTab() {
  if (ImGui::BeginTabItem("Consol")) {
    console.Draw();
    ImGui::EndTabItem();
  }
}

void TestApp::init_GL() {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  /////////////////////////
  // Create and bind VBO //
  /////////////////////////
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  ////////////////////////////////////////////////////////////////////
  // Create program for drawing cube, create VBOs and copy the data //
  ////////////////////////////////////////////////////////////////////
  cube_program = LoadShaders("vertex_shader.vs", "fragment_shader.fs");

  // Actually a cylinder with 20 triangles
  num_indices = 14;

  const GLfloat cubeVertices[] = {
     -1.0f, -1.0f,  1.0f,
      1.0f, -1.0f,  1.0f,
     -1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,
     -1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
     -1.0f,  1.0f, -1.0f,
      1.0f,  1.0f, -1.0f
  };

  const GLfloat cubeColors[] = {
      0.0f,  0.0f,  0.0f,
      1.0f,  0.0f,  0.0f,
      1.0f,  1.0f,  0.0f,
      1.0f,  1.0f,  1.0f,
      0.0f,  1.0f,  1.0f,
      0.0f,  0.0f,  1.0f,
      0.0f,  1.0f,  0.0f,
      1.0f,  0.0f,  1.0f
  };

  const GLubyte cubeIndices[] = {
      0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1
  };
  
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
  glGenBuffers(1, &colorbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeColors), cubeColors, GL_STATIC_DRAW);

  glGenBuffers(1, &elementbuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
  glBindVertexArray(0);

  ////////////////////////////////////////////////////////////////////////
  // Create and bind framebuffer, attach a depth buffer to it           //
  // Create the texture to render to, and attach it to the framebuffer  //
  ////////////////////////////////////////////////////////////////////////
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  glGenRenderbuffers(1, &depthbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, resx, resy);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer);

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resx, resy, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
  GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers(1, DrawBuffers);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("Error in setting up the framebuffer\n");
  }
}

void TestApp::draw_cube() {
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

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glViewport(0, 0, textureX, textureY);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(cube_program);

  /////////////////////////////////////////////////////////////////
  // View matrix is identity matrix for 2D,                      //
  // camera movement put in orthogonal projection matrix instead //
  /////////////////////////////////////////////////////////////////
  glm::mat4 View = glm::rotate(glm::mat4(1.0f), rotateX, glm::vec3(1.0, 0.0, 0.0))
                    *glm::rotate(glm::mat4(1.0f), rotateY, glm::vec3(0.0, 1.0, 0.0));
  glm::mat4 Projection = glm::ortho<float>(cam_x, cam_x + cam_width, cam_y, cam_y + cam_height, -10, 10);
  //mat4 View = view(vec3(1,0,0), vec3(0,1,0), vec3(0,0,-1), vec3(cam_x,cam_y,6));
  //mat4 Projection = projection(60, resx/float(resy), 0.01, 10.0);

  ////////////////////////////////
  // same RNG seed every frame  //
  // "minimal standard" LCG RNG //
  ////////////////////////////////
  glm::mat4 Model = glm::mat4(1.0f);

  glm::mat4 MVP = Projection * View * Model;
  glUniformMatrix4fv(glGetUniformLocation(cube_program, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

  // Draw!
  glBindVertexArray(VertexArrayID);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  glVertexAttribDivisor(0, 0);
  glVertexAttribDivisor(1, 0);
  glDrawElements(GL_TRIANGLE_STRIP, num_indices, GL_UNSIGNED_BYTE, NULL);
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glBindVertexArray(0);
  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void TestApp::cube_GUI() {
  /////////////////////////////////////////////////////
  // Get available size of the current window (dock) //
  // and adjust the texture size if it has changed   //
  // keep cam_height constant                        //
  /////////////////////////////////////////////////////
  ImVec2 size = ImGui::GetContentRegionAvail();
  if (textureX != size.x || textureY != size.y) {
    textureX = size.x;
    textureY = size.y;
    cam_width = cam_height * textureX / float(textureY);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureX, textureY, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, textureX, textureY);
  }

  draw_cube();

  ImGuiIO& io = ImGui::GetIO();
  static ImVec2 imagePos = ImVec2(textureX / 2, textureY / 2); // If mouse cursor is outside the screen, use center of image as zoom point
  ImVec2 cursorPos = ImGui::GetCursorScreenPos();          // "cursor" is where imgui will draw the image
  ImVec2 mousePos = io.MousePos;

  //////////////////////////////////////////////////////
  // Draw the image/texture, filling the whole window //
  //////////////////////////////////////////////////////
  //ImGui::Image(reinterpret_cast<ImTextureID>(texture), size, ImVec2(0, 0), ImVec2(1, -1));
  ImGui::Image((ImTextureID)((size_t)texture), size, ImVec2(0, 0), ImVec2(1, -1));


  bool isHovered = ImGui::IsItemHovered();
  bool isLeftClicked = ImGui::IsMouseClicked(0, false);
  bool isRightClicked = ImGui::IsMouseClicked(1, false);
  bool isFocused = ImGui::IsWindowFocused();

  ////////////////////////////////////////////////////////////////////////////////////
  // So we can move the camera while the mouse cursor is outside the docking window //
  // but the button is not released yet                                             //
  ////////////////////////////////////////////////////////////////////////////////////
  static bool clickedLeftWhileHovered = false;
  static bool clickedRightWhileHovered = false;
  if (isHovered && isLeftClicked) {
    clickedLeftWhileHovered = true;
  }
  if (isHovered && isRightClicked) {
    clickedRightWhileHovered = true;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Only do input if the window containing the image is in focus (activaed) //
  /////////////////////////////////////////////////////////////////////////////
  if (isFocused) {
    ImVec2 mouseDelta = io.MouseDelta;

    if (clickedLeftWhileHovered) {
      clickedLeftWhileHovered = io.MouseDown[0];
      if (io.MouseDown[0]) {
        cam_x -= mouseDelta.x / size.x*cam_width;
        cam_y += mouseDelta.y / size.y*cam_height;
      }
    }
    if (clickedRightWhileHovered) {
      clickedRightWhileHovered = io.MouseDown[1];
      if (io.MouseDown[1]) {
        float degreesPerPixel = 0.05f;
        rotateX += degreesPerPixel * mouseDelta.y;
        rotateY += degreesPerPixel * mouseDelta.x;
      }
    }

    float speed = 3; // 3 units per second
    if (glfwGetKey(this->GetWindow(), GLFW_KEY_LEFT_CONTROL)) speed *= 0.1; // slow
    if (glfwGetKey(this->GetWindow(), GLFW_KEY_LEFT_SHIFT))   speed *= 10.0;  // fast

    float deltaMoveForward = speed * (glfwGetKey(this->GetWindow(), GLFW_KEY_W) - glfwGetKey(this->GetWindow(), GLFW_KEY_S));
    float deltaMoveRight = speed * (glfwGetKey(this->GetWindow(), GLFW_KEY_D) - glfwGetKey(this->GetWindow(), GLFW_KEY_A));
    float deltaMoveUp = speed * (glfwGetKey(this->GetWindow(), GLFW_KEY_E) - glfwGetKey(this->GetWindow(), GLFW_KEY_Q));


    // move camera...
    float dt = io.DeltaTime;
    cam_x -= dt * deltaMoveRight;
    cam_y -= dt * deltaMoveUp;

    // Zoom...
    if (isHovered) {
      imagePos = ImVec2(mousePos.x - cursorPos.x, mousePos.y - cursorPos.y);
    }

    ///////////////////////////////////////////////////////////
    // Zoom by keeping the mouse cursor at the same location //
    // in world coordinates before and after zooming         //
    ///////////////////////////////////////////////////////////
    float zoomFactor = glm::pow(0.95f, dt*deltaMoveForward);
    float xend = cam_x + imagePos.x*(cam_width) / textureX;
    float yend = cam_y + (textureY - imagePos.y)*cam_height / textureY;
    cam_x = (1.0 - zoomFactor)*xend + zoomFactor * cam_x;
    cam_y = (1.0 - zoomFactor)*yend + zoomFactor * cam_y;

    cam_width *= zoomFactor;
    cam_height *= zoomFactor;

    if (ImGui::IsMouseDoubleClicked(0)) {
      fullscreen = !fullscreen;
    }
  }
}

std::string TestApp::readFile(const char *filename) {
  /////////////////////////////////////////////////////////////
  // Read content of "filename" and return it as a c-string. //
  /////////////////////////////////////////////////////////////
  std::string str;
  std::cout << "Reading " << filename << std::endl;
  std::ifstream in(filename, std::ios::ate);
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

void TestApp::CompileShader(const char *file_path, GLuint ShaderID) {
  GLint Result = GL_FALSE;
  int InfoLogLength;

  std::string shaderString = readFile(file_path);
  const GLchar *ShaderCode = shaderString.c_str();

  // Compile Shader
  printf("Compiling shader : %s\n", file_path);
  glShaderSource(ShaderID, 1, &ShaderCode, NULL);
  glCompileShader(ShaderID);

  // Check Shader
  glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);

  if (Result == GL_FALSE) {
    char ShaderErrorMessage[9999];
    glGetShaderInfoLog(ShaderID, InfoLogLength, NULL, ShaderErrorMessage);
    printf("%s", ShaderErrorMessage);
  }
}

GLuint TestApp::LoadShaders(const char *vertex_file_path, const char *fragment_file_path) {
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


} // inline namespace opengl3

} // namespace vktuto