#include <iostream>
#include <random>

#include "test_app.h"

namespace vktuto {

inline namespace opengl3 {

TestApp::TestApp() {
  Initialize();
}

TestApp::~TestApp() {
  Cleanup();
}

void TestApp::Initialize() {
  ChangeOutData();
  window_flags_ = SetupWindowFlags();
}

void TestApp::Update() {
  if (show_main_window_) ShowMainWindow();
}

void TestApp::Cleanup() {
}

void TestApp::ChangeOutData() {
  int row_size = num_para_u - 1, col_size = num_para_v - 1;
  if (!nurbs::internal::SurfaceIsValid(
          srf.degree_u, srf.degree_v,
          srf.knots_u, srf.knots_v,
          srf.control_points, srf.weights) || 
      row_size <= 0 || col_size <= 0) {
    return;
  }

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(0, 1);

  auto & vertices = GetVertices();
  vertices = surface;

  auto & colors = GetColors();
  colors.resize(surface.size());
  for (auto & color : colors) {
    color = glm::vec3(dis(gen), dis(gen), dis(gen));
  }

  auto & indices = GetIndices();
  indices.resize(6 * row_size * col_size);
  std::vector<GLuint> mesh(6);
  for (size_t u_idx = 0; u_idx < num_para_u; u_idx++) {
    if (u_idx == row_size) continue;
    for (size_t v_idx = 0; v_idx < num_para_v; v_idx++) {
    if (v_idx == col_size) continue;
      unsigned int idx = num_para_v * u_idx + v_idx;
      unsigned int loc = col_size * (u_idx % row_size) + (v_idx % col_size);
      mesh = { idx, idx + 1, idx + num_para_v,
               idx + 1 , idx + 1 + num_para_v, idx + num_para_v };
      std::copy(mesh.begin(), mesh.end(), indices.begin() + 6 * loc);
    }
  }

  BindBuffers();
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
  ImGuiIO &io = ImGui::GetIO();
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(io.DisplaySize);
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
  static unsigned int counter_u = 2;
  static unsigned int counter_v = 2;
  static unsigned int num_con_point_u = 5;
  static unsigned int num_con_point_v = 5;

  // Degree and Control points
  {
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Degree of direction U:");
    ImGui::SameLine();

    float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
    ImGui::PushButtonRepeat(true);
    if (ImGui::ArrowButton("##left_u", ImGuiDir_Left)) {
      if (counter_u != 1) --counter_u;
    }
    ImGui::SameLine(0.0f, spacing);
    if (ImGui::ArrowButton("##right_u", ImGuiDir_Right)) {
      if (counter_u != 7) ++counter_u;
    }
    ImGui::PopButtonRepeat();
    ImGui::SameLine();
    ImGui::Text("%d", counter_u);

    ImGui::Text("Degree of direction V:");
    ImGui::SameLine();

    ImGui::PushButtonRepeat(true);
    if (ImGui::ArrowButton("##left_v", ImGuiDir_Left)) {
      if (counter_v != 1) --counter_v;
    }
    ImGui::SameLine(0.0f, spacing);
    if (ImGui::ArrowButton("##right_v", ImGuiDir_Right)) {
      if (counter_v != 7) ++counter_v;
    }
    ImGui::PopButtonRepeat();
    ImGui::SameLine();
    ImGui::Text("%d", counter_v);

    ImGui::Text("The # of control points for U:");
    ImGui::SameLine();

    ImGui::PushButtonRepeat(true);
    if (ImGui::ArrowButton("##left_cp_u", ImGuiDir_Left)) {
      if (num_con_point_u != 2) --num_con_point_u;
    }
    ImGui::SameLine(0.0f, spacing);
    if (ImGui::ArrowButton("##right_cp_u", ImGuiDir_Right)) {
      if (num_con_point_u != 10) ++num_con_point_u;
    }
    ImGui::PopButtonRepeat();
    ImGui::SameLine();
    ImGui::Text("%d", num_con_point_u);

    ImGui::Text("The # of control points for V:");
    ImGui::SameLine();

    ImGui::PushButtonRepeat(true);
    if (ImGui::ArrowButton("##left_cp_v", ImGuiDir_Left)) {
      if (num_con_point_v != 2) --num_con_point_v;
    }
    ImGui::SameLine(0.0f, spacing);
    if (ImGui::ArrowButton("##right_cp_v", ImGuiDir_Right)) {
      if (num_con_point_v != 10) ++num_con_point_v;
    }
    ImGui::PopButtonRepeat();
    ImGui::SameLine();
    ImGui::Text("%d", num_con_point_v);
  }

  // Make surface
  if (ImGui::Button("Make surface"))
  {
    srf.degree_u = 3;
    srf.degree_v = 3;
    srf.knots_u = { 0, 0, 0, 0, 0.5, 1, 1, 1, 1 };
    srf.knots_v = { 0, 0, 0, 0, 0.5, 1, 1, 1, 1 };
    srf.control_points = { 5, 5,
        { glm::vec3(0.5, 0.0, 0.0), glm::vec3(1.0, 0.0, 0.0), glm::vec3(1.3, 0.0, 0.0), glm::vec3(2.0, 0.0, 0.0), glm::vec3(2.8, 0.0, 0.0),
          glm::vec3(0.4, 1.0, 0.0), glm::vec3(1.0, 1.0, 0.0), glm::vec3(2.3, 1.0, 0.2), glm::vec3(3.0, 1.0, 0.0), glm::vec3(3.5, 1.0, 0.0),
          glm::vec3(0.0, 2.0, 0.0), glm::vec3(0.7, 2.0, 0.0), glm::vec3(1.3, 2.0, 1.2), glm::vec3(2.0, 2.0, 0.0), glm::vec3(2.8, 2.0, 0.0),
          glm::vec3(0.4, 3.0, 0.0), glm::vec3(1.0, 3.0, 0.0), glm::vec3(2.3, 3.0, 0.2), glm::vec3(3.0, 3.0, 0.0), glm::vec3(3.5, 3.0, 0.0),
          glm::vec3(0.0, 4.0, 0.0), glm::vec3(0.7, 4.0, 0.0), glm::vec3(1.3, 4.0, 0.0), glm::vec3(2.0, 4.0, 0.0), glm::vec3(2.8, 4.0, 0.0)
        }
    };
    srf.weights = { 5, 5,
        { 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1,
          1, 1, 1, 1, 1,
          1, 1, 1, 1, 1,
          1, 1, 1, 1, 1
        }
    };

    if (nurbs::internal::SurfaceIsValid(
        srf.degree_u, srf.degree_v,
        srf.knots_u, srf.knots_v,
        srf.control_points, srf.weights)) {
      float interval_u = 0.01f, interval_v = 0.01f;
      num_para_u = 1 / interval_u + 1;
      num_para_v = 1 / interval_v + 1;
      surface.resize(num_para_u * num_para_v);
      for (size_t u_idx = 0; u_idx < num_para_u; ++u_idx) {
        float para_u = interval_u * u_idx;
          for (size_t v_idx = 0; v_idx < num_para_v; ++v_idx) {
            float para_v = interval_v * v_idx;
            glm::vec3 pt = nurbs::SurfacePoint(srf, para_u, para_v);
            surface.at(num_para_v * u_idx + v_idx) = pt;
          }
      }

      ChangeOutData();

      nurbs::SurfaceSaveOBJ("surface.txt", srf);
    }
    else {
      std::cerr << "Parameters(degree and control points) are wrong!" << std::endl;
    }
  }

  // Load surface
  if (ImGui::Button("Load surface"))
  {
    std::string file_name = "surface.txt";
    auto srf2 = nurbs::SurfaceReadOBJ<3, float>(file_name.c_str());

    if (nurbs::internal::SurfaceIsValid(
      srf2.degree_u, srf2.degree_v,
      srf2.knots_u, srf2.knots_v,
      srf2.control_points, srf2.weights)) {
      float interval_u = 0.01f, interval_v = 0.01f;
      num_para_u = 1 / interval_u + 1;
      num_para_v = 1 / interval_v + 1;
      surface.resize(num_para_u * num_para_v);
      for (size_t u_idx = 0; u_idx < num_para_u; ++u_idx) {
        float para_u = interval_u * u_idx;
        for (size_t v_idx = 0; v_idx < num_para_v; ++v_idx) {
          float para_v = interval_v * v_idx;
          glm::vec3 pt = nurbs::SurfacePoint(srf2, para_u, para_v);
          surface.at(num_para_v * u_idx + v_idx) = pt;
        }
      }

      srf = srf2;
      ChangeOutData();
    }
    else {
      std::cerr << "Parameters(degree and control points) are wrong!" << std::endl;
    }
  }

  // Set draw mode
  {
    static int draw_mode = 2;
    ImGui::RadioButton("Polygon", &draw_mode, 2); ImGui::SameLine();
    ImGui::RadioButton("Wireframe", &draw_mode, 1); ImGui::SameLine();
    ImGui::RadioButton("Point", &draw_mode, 0);
    ChangeDrawMode(draw_mode);
  }
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
    CanvasContext();
    ImGui::EndTabItem();
  }
}

void TestApp::ConsolTab() {
  if (ImGui::BeginTabItem("Consol")) {
    console.Draw();
    ImGui::EndTabItem();
  }
}

void TestApp::CanvasContext() {
  /////////////////////////////////////////////////////
  // Get available size of the current window (dock) //
  // and adjust the texture size if it has changed   //
  // keep cam_height constant                        //
  /////////////////////////////////////////////////////
  ImVec2 size = ImGui::GetContentRegionAvail();
  if (GetTextrueX() != size.x || GetTextrueY() != size.y) {
    GetTextrueX() = size.x;
    GetTextrueY() = size.y;
    GetCamW() = GetCamH() * GetTextrueX() / float(GetTextrueY());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->GetTexture());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GetTextrueX(), GetTextrueY(), 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glBindFramebuffer(GL_FRAMEBUFFER, this->GetFBO());
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, GetTextrueX(), GetTextrueY());
  }

  ImGuiIO& io = ImGui::GetIO();
  static ImVec2 image_pos = ImVec2(GetTextrueX() / 2, GetTextrueY() / 2); // If mouse cursor is outside the screen, use center of image as zoom point
  ImVec2 cursor_pos = ImGui::GetCursorScreenPos();          // "cursor" is where imgui will draw the image
  ImVec2 mouse_pos = io.MousePos;

  //////////////////////////////////////////////////////
  // Draw the image/texture, filling the whole window //
  //////////////////////////////////////////////////////
  //ImGui::Image(reinterpret_cast<ImTextureID>(texture), size, ImVec2(0, 0), ImVec2(1, -1));
  ImGui::Image((ImTextureID)((size_t)this->GetTexture()), size, ImVec2(0, 0), ImVec2(1, -1));


  bool is_hovered = ImGui::IsItemHovered();
  bool is_left_clicked = ImGui::IsMouseClicked(0, false);
  bool is_right_clicked = ImGui::IsMouseClicked(1, false);
  bool is_focused = ImGui::IsWindowFocused();

  ////////////////////////////////////////////////////////////////////////////////////
  // So we can move the camera while the mouse cursor is outside the docking window //
  // but the button is not released yet                                             //
  ////////////////////////////////////////////////////////////////////////////////////
  static bool clicked_left_while_hovered = false;
  static bool clicked_right_while_hovered = false;
  if (is_hovered && is_left_clicked) {
    clicked_left_while_hovered = true;
  }
  if (is_hovered && is_right_clicked) {
    clicked_right_while_hovered = true;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Only do input if the window containing the image is in focus (activaed) //
  /////////////////////////////////////////////////////////////////////////////
  if (is_focused) {
    ImVec2 mouse_delta = io.MouseDelta;

    if (clicked_left_while_hovered) {
      clicked_left_while_hovered = io.MouseDown[0];
      if (io.MouseDown[0]) {
        GetCamX() -= mouse_delta.x / size.x*GetCamW();
        GetCamY() += mouse_delta.y / size.y*GetCamH();
      }
    }
    if (clicked_right_while_hovered) {
      clicked_right_while_hovered = io.MouseDown[1];
      if (io.MouseDown[1]) {
        float degreesPerPixel = 0.05f;
        GetRotX() += degreesPerPixel * mouse_delta.y;
        GetRotY() += degreesPerPixel * mouse_delta.x;
      }
    }

    float speed = 3; // 3 units per second
    if (glfwGetKey(this->GetWindow(), GLFW_KEY_LEFT_CONTROL)) speed *= 0.1; // slow
    if (glfwGetKey(this->GetWindow(), GLFW_KEY_LEFT_SHIFT))   speed *= 10.0;  // fast

    float delta_move_forward = speed * (glfwGetKey(this->GetWindow(), GLFW_KEY_W) - glfwGetKey(this->GetWindow(), GLFW_KEY_S));
    float delta_move_right = speed * (glfwGetKey(this->GetWindow(), GLFW_KEY_D) - glfwGetKey(this->GetWindow(), GLFW_KEY_A));
    float delta_move_up = speed * (glfwGetKey(this->GetWindow(), GLFW_KEY_E) - glfwGetKey(this->GetWindow(), GLFW_KEY_Q));


    // Move camera...
    float dt = io.DeltaTime;
    GetCamX() -= dt * delta_move_right;
    GetCamY() -= dt * delta_move_up;

    // Zoom...
    if (is_hovered) {
      image_pos = ImVec2(mouse_pos.x - cursor_pos.x, mouse_pos.y - cursor_pos.y);
    }

    ///////////////////////////////////////////////////////////
    // Zoom by keeping the mouse cursor at the same location //
    // in world coordinates before and after zooming         //
    ///////////////////////////////////////////////////////////
    float zoom_factor = glm::pow(0.95f, dt*delta_move_forward);
    float x_end = GetCamX() + image_pos.x*(GetCamW()) / GetTextrueX();
    float y_end = GetCamY() + (GetTextrueY() - image_pos.y)*GetCamH() / GetTextrueY();
    GetCamX() = (1.0 - zoom_factor)*x_end + zoom_factor * GetCamX();
    GetCamY() = (1.0 - zoom_factor)*y_end + zoom_factor * GetCamY();

    GetCamW() *= zoom_factor;
    GetCamH() *= zoom_factor;

    if (ImGui::IsMouseDoubleClicked(0)) {
      IsFullScreen() = !IsFullScreen();
    }
  }
}

nurbs::RationalSurface3f TestApp::MakeSurface(
    unsigned int degree_u,
    unsigned int degree_v,
    std::vector<float> knots_u,
    std::vector<float> knots_v,
    nurbs::array2<glm::vec3> control_points,
    nurbs::array2<float> weigths) {
  nurbs::RationalSurface3f srf;
  srf.degree_u = degree_u;
  srf.degree_v = degree_v;
  srf.knots_u = knots_u;
  srf.knots_v = knots_v;
  srf.control_points = control_points;
  srf.weights = weigths;
  return srf;
}

} // inline namespace opengl3

} // namespace vktuto