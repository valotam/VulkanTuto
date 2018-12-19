#include <iostream>

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
  GetVertices() = {
    glm::vec3(-1.0f, -1.0f,  1.0f),
    glm::vec3( 1.0f, -1.0f,  1.0f),
    glm::vec3(-1.0f,  1.0f,  1.0f),
  };

  GetColors() = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(1.0f,  0.0f,  0.0f),
    glm::vec3(1.0f,  1.0f,  0.0f),
  };

  GetIndices() = {
      0, 1, 2
  };

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

} // inline namespace opengl3

} // namespace vktuto