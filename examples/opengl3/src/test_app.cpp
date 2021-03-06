#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <random>
#include <functional>

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
          surface_primitive.degree_u, surface_primitive.degree_v,
          surface_primitive.knots_u, surface_primitive.knots_v,
          surface_primitive.control_points, surface_primitive.weights) ||
      row_size <= 0 || col_size <= 0) {
    return;
  }

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(0, 1);

  auto & vertices = GetVertices();
  vertices = surface_points;

  auto & colors = GetColors();
  colors.resize(surface_points.size());
  for (auto & color : colors) {
    color = glm::vec3(1.0, 0.6, 0.1);//glm::vec3(dis(gen), dis(gen), dis(gen));
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

void TestApp::ChangeOutData2() {
  auto &positions = GetVertices2();
  positions.resize(curve_points.size() + curve_tangents.size()
                    + curve_normals.size() + curve_binormals.size());
  for (size_t idx = 0; idx < positions.size(); idx++) {
    if (idx < curve_points.size()) {
      positions.at(idx) = curve_points.at(idx);
    } else if (idx < curve_points.size() + curve_tangents.size()) {
      size_t ii = idx - curve_points.size();
      positions.at(idx) = curve_points.at(ii) + curve_tangents.at(ii);
    } else if (idx < curve_points.size() + curve_tangents.size() + curve_normals.size()) {
      size_t ii = idx - curve_points.size() - curve_tangents.size();
      positions.at(idx) = curve_points.at(ii) - curve_normals.at(ii);
    } else {
      size_t ii = idx - curve_points.size() - curve_tangents.size() - curve_normals.size();
      positions.at(idx) = curve_points.at(ii) + curve_binormals.at(ii);
    }
  }

  auto &colors = GetColors2();
  colors.resize(positions.size());
  for (size_t idx = 0; idx < colors.size(); idx++) {
    if (idx < curve_points.size()) {
      colors.at(idx) = glm::vec3(0.5, 0.95, 0.45);
    } else if (idx < curve_points.size() + curve_tangents.size()) {
      colors.at(idx) = glm::vec3(1.0, 0.5, 0.5);
    } else if (idx < curve_points.size() + curve_tangents.size() + curve_normals.size()) {
      colors.at(idx) = glm::vec3(1.0, 1.0, 0.0);
    } else {
      colors.at(idx) = glm::vec3(0.18, 0.77, 0.71);
    }
  }

  auto &indices = GetIndices2();
  indices.clear();
  size_t total_size = curve_points.size() - 1;
  if (show_tangent)   total_size += curve_tangents.size();
  if (show_normal)    total_size += curve_normals.size();
  if (show_binormal)  total_size += curve_binormals.size();
  indices.reserve(2 * total_size);
  for (unsigned int idx = 0; idx < curve_points.size(); idx++) {
    unsigned int ii;
    if (idx < curve_points.size() - 1) {
      indices.push_back(idx); indices.push_back(idx + 1);
    }
    if (show_tangent) {
      ii = idx + curve_points.size();
      indices.push_back(idx); indices.push_back(ii);
    }
    if (show_normal) {
      ii = idx + curve_points.size() + curve_tangents.size();
      indices.push_back(idx); indices.push_back(ii);
    }
    if (show_binormal) {
      ii = idx + curve_points.size() + curve_tangents.size() + curve_normals.size();
      indices.push_back(idx); indices.push_back(ii);
    }
  }

  BindBuffers2();
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
  //window_flags |= ImGuiWindowFlags_NoNav;
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
  if (ImGui::CollapsingHeader("Surface")) {
    static bool cp_changed = true, degree_changed = true;
    // Pick 4 boundary points
    if (ImGui::TreeNode("Set 4 corner points")) {
      ImGui::Columns(4, "##pick-four-boundary-points");
      ImGui::Separator();
      ImGui::Text("X"); ImGui::NextColumn();
      ImGui::Text("Y"); ImGui::NextColumn();
      ImGui::Text("Z"); ImGui::NextColumn();
      ImGui::Text(ICON_FA_LOCK "/" ICON_FA_LOCK_OPEN); ImGui::NextColumn();
      ImGui::Separator();
      static int selected = -1;
      static float fx = 0.0f, fy = 0.0f, fz = 0.0f;
      static bool button_clicked[4] = { false, false, false, false };
      for (int i = 0; i < 4; i++) {
        std::stringstream idx; idx << i;
        std::string selectable_name("##dummy-bound");
        std::string confirm_name(ICON_FA_LOCK_OPEN "##");
        std::string change_name(ICON_FA_LOCK "##");
        selectable_name.append(idx.str());
        confirm_name.append(idx.str());
        change_name.append(idx.str());
        if (button_clicked[i]) {
          ImGui::InputFloat((selectable_name + "x").c_str(), &boundary[i].x, 0.0f); ImGui::NextColumn();
          ImGui::InputFloat((selectable_name + "y").c_str(), &boundary[i].y, 0.0f); ImGui::NextColumn();
          ImGui::InputFloat((selectable_name + "z").c_str(), &boundary[i].z, 0.0f); ImGui::NextColumn();
          if (ImGui::Button(confirm_name.c_str())) {
            button_clicked[i] = false;
            cp_changed = true;
          }
        }
        else {
          idx << std::fixed << std::setprecision(2);
          idx.str(std::string()); idx << boundary[i].x;
          ImGui::Text(idx.str().c_str()); ImGui::NextColumn();
          idx.str(std::string()); idx << boundary[i].y;
          ImGui::Text(idx.str().c_str()); ImGui::NextColumn();
          idx.str(std::string()); idx << boundary[i].z;
          ImGui::Text(idx.str().c_str()); ImGui::NextColumn();
          if (ImGui::Button(change_name.c_str())) button_clicked[i] = true;
        }
        ImGui::NextColumn();
      }
      ImGui::Columns(1);
      ImGui::Separator();
      
      ImGui::TreePop();
    }

    // The number of control points
    {
      ImGui::PushFont(GetBoldFont());
      ImGui::Text("The number of C.P.");
      ImGui::PopFont(); ImGui::SameLine();
      ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() * 0.3f);
      cp_changed |= ImGui::DragInt("##cp_u", &num_surface_con_point_u, 0.1, 2, 20, "U: %d"); ImGui::SameLine();
      cp_changed |= ImGui::DragInt("##cp_v", &num_surface_con_point_v, 0.1, 2, 20, "V: %d");
      ImGui::PopItemWidth();
    }

    // Make control points
    {
      if (cp_changed) {
        surface_control_points.resize(num_surface_con_point_u, num_surface_con_point_v);
        for (size_t v_idx = 0; v_idx < surface_control_points.cols(); v_idx++) {
          float vv = static_cast<float>(v_idx) / (surface_control_points.cols() - 1);
          for (size_t u_idx = 0; u_idx < surface_control_points.rows(); u_idx++) {
            float uu = static_cast<float>(u_idx) / (surface_control_points.rows() - 1);
            surface_control_points(u_idx, v_idx) =
              boundary[0] * (1 - uu)  * (1 - vv) +
              boundary[1] * (1 - uu)  * vv +
              boundary[2] * uu        * (1 - vv) +
              boundary[3] * uu        * vv;
          }
        }
      }
    }

    // Customize control points
    if (ImGui::TreeNode("Customize control points")) {
      ImGui::BeginGroup();
      ImGui::BeginChild("##cp-list", ImVec2(0, 200), true);
      for (size_t u_idx = 0; u_idx < surface_control_points.rows(); u_idx++) {
        for (size_t v_idx = 0; v_idx < surface_control_points.cols(); v_idx++) {
          std::stringstream idx;
          idx << u_idx << v_idx;
          std::string dummy_name("##dummy");
          dummy_name.append(idx.str());
          ImGui::Text("C.P.(%d, %d)", u_idx, v_idx);
          ImGui::SameLine();
          cp_changed |= ImGui::InputFloat3(dummy_name.c_str(), surface_control_points(u_idx, v_idx).data.data);
        }
      }
      ImGui::EndChild();
      ImGui::EndGroup();

      ImGui::TreePop();
    }

    // Degrees
    {
      ImVec4 color(ImGui::GetStyleColorVec4(ImGuiCol_Text));
      if (degree_u >= num_surface_con_point_u || degree_v >= num_surface_con_point_v)
        color = ImGui::GetStyleColorVec4(ImGuiCol_PlotLinesHovered);
      ImGui::PushFont(GetBoldFont());
      ImGui::TextColored(color, "Degree");
      ImGui::PopFont(); ImGui::SameLine();
      ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() * 0.25f);
      degree_changed |= ImGui::DragInt("##degree_u", &degree_u, 0.1, 1, num_surface_con_point_u - 1, "U: %d"); ImGui::SameLine();
      degree_changed |= ImGui::DragInt("##degree_v", &degree_v, 0.1, 1, num_surface_con_point_v - 1, "V: %d");
      ImGui::PopItemWidth();
    }

    // Make knots
    if (cp_changed || degree_changed) {
      auto & knots_u = surface_primitive.knots_u;
      knots_u.resize(surface_control_points.rows() + degree_u + 1);
      auto & knots_v = surface_primitive.knots_v;
      knots_v.resize(surface_control_points.cols() + degree_v + 1);
      for (size_t u_idx = 0; u_idx < knots_u.size(); u_idx++) {
        if (u_idx < degree_u + 1) {
          knots_u.at(u_idx) = 0;
        }
        else if (u_idx < surface_control_points.rows()) {
          knots_u.at(u_idx) = static_cast<float>(u_idx - degree_u) /
            (surface_control_points.rows() - degree_u);
        }
        else {
          knots_u.at(u_idx) = 1;
        }
      }
      for (size_t v_idx = 0; v_idx < knots_v.size(); v_idx++) {
        if (v_idx < degree_v + 1) {
          knots_v.at(v_idx) = 0;
        }
        else if (v_idx < surface_control_points.cols()) {
          knots_v.at(v_idx) = static_cast<float>(v_idx - degree_v) /
            (surface_control_points.cols() - degree_v);
        }
        else {
          knots_v.at(v_idx) = 1;
        }
      }

      cp_changed = degree_changed = false;
    }

    // Customize knots
    if (ImGui::TreeNode("Customize UV knots")) {
      //bool track_line_u = false, track_line_v = false;
      //static int track_uknot = 0, track_vknot = 0;
      ImGui::BeginGroup();
      //ImGui::PushFont(GetBoldFont());
      //ImGui::Text("Customize UV knots");
      //ImGui::PopFont();
      //ImGui::Text("Choose a knot where"); ImGui::SameLine();
      //ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() * 0.3f);
      //track_line_u |= ImGui::DragInt("##line_uknots", &track_uknot, 0.25f, 0, surface_primitive.knots_u.size() - 1, "U = %d"); ImGui::SameLine();
      //track_line_v |= ImGui::DragInt("##line_vknots", &track_vknot, 0.25f, 0, surface_primitive.knots_v.size() - 1, "V = %d");
      //ImGui::PopItemWidth();
      ImGui::Columns(2);
      ImGui::BeginChild("##uknots-list", ImVec2(0, 150), true);
      for (size_t u_idx = 0; u_idx < surface_primitive.knots_u.size(); u_idx++) {
        std::stringstream idx;
        idx << u_idx;
        std::string dummy_name("##dummy_uknots");
        dummy_name.append(idx.str());
        //if (u_idx == track_uknot) {
        ImGui::Text("U Knot(%d)", u_idx);
        ImGui::SameLine();
        ImGui::InputFloat(dummy_name.c_str(), &surface_primitive.knots_u.at(u_idx));
        //  //if (track_line_u) ImGui::SetScrollHereY();
        //}
        //else {
        //  ImGui::Text("U Knot(%d)", u_idx);
        //  ImGui::SameLine();
        //  ImGui::InputFloat(dummy_name.c_str(), &surface_primitive.knots_u.at(u_idx));
        //}
      }
      ImGui::EndChild(); ImGui::NextColumn();
      ImGui::BeginChild("##vknots-list", ImVec2(0, 150), true);
      for (size_t v_idx = 0; v_idx < surface_primitive.knots_v.size(); v_idx++) {
        std::stringstream idx;
        idx << v_idx;
        std::string dummy_name("##dummy_vknots");
        dummy_name.append(idx.str());
        //if (v_idx == track_vknot) {
        ImGui::Text("V Knot(%d)", v_idx);
        ImGui::SameLine();
        ImGui::InputFloat(dummy_name.c_str(), &surface_primitive.knots_v.at(v_idx));
        //  if (track_line_v) ImGui::SetScrollHereY();
        //}
        //else {
        //  ImGui::Text("V Knot(%d)", v_idx);
        //  ImGui::SameLine();
        //  ImGui::InputFloat(dummy_name.c_str(), &surface_primitive.knots_v.at(v_idx));
        //}
      }
      ImGui::EndChild(); ImGui::NextColumn();
      ImGui::Columns(1);
      ImGui::EndGroup();

      ImGui::TreePop();
    }

    // Make surface
    if (ImGui::Button("Make surface"))
    {
      nurbs::array2<float> wei = { surface_control_points.rows(), surface_control_points.cols(), { 1, } };
      MakeSurface(degree_u, degree_v,
        surface_primitive.knots_u, surface_primitive.knots_v,
        surface_control_points, wei);

      if (nurbs::internal::SurfaceIsValid(
        surface_primitive.degree_u, surface_primitive.degree_v,
        surface_primitive.knots_u, surface_primitive.knots_v,
        surface_primitive.control_points, surface_primitive.weights)) {
        float interval_u = 0.01f, interval_v = 0.01f;
        num_para_u = 1 / interval_u + 1;
        num_para_v = 1 / interval_v + 1;
        surface_points.resize(num_para_u * num_para_v);
        for (size_t u_idx = 0; u_idx < num_para_u; ++u_idx) {
          float para_u = interval_u * u_idx;
          for (size_t v_idx = 0; v_idx < num_para_v; ++v_idx) {
            float para_v = interval_v * v_idx;
            glm::vec3 pt = nurbs::SurfacePoint(surface_primitive, para_u, para_v);
            surface_points.at(num_para_v * u_idx + v_idx) = pt;
          }
        }

        ChangeOutData();

        //  nurbs::SurfaceSaveOBJ("surface.txt", srf);
      }
      else {
        std::cerr << "Parameters(degree and control points) are wrong!" << std::endl;
      }
    }
    ImGui::SameLine();

    // Clear surface
    if (ImGui::Button("Clear surface")) {
      GetVertices() = std::vector<glm::vec3>();
      GetColors() = std::vector<glm::vec3>();
      GetIndices() = std::vector<GLuint>();
      BindBuffers();
    }

    // Load surface
    //if (ImGui::Button("Load surface"))
    //{
      //std::string file_name = "surface.txt";
      //auto srf2 = nurbs::SurfaceReadOBJ<3, float>(file_name.c_str());

      //if (nurbs::internal::SurfaceIsValid(
      //  srf2.degree_u, srf2.degree_v,
      //  srf2.knots_u, srf2.knots_v,
      //  srf2.control_points, srf2.weights)) {
      //  float interval_u = 0.01f, interval_v = 0.01f;
      //  num_para_u = 1 / interval_u + 1;
      //  num_para_v = 1 / interval_v + 1;
      //  surface.resize(num_para_u * num_para_v);
      //  for (size_t u_idx = 0; u_idx < num_para_u; ++u_idx) {
      //    float para_u = interval_u * u_idx;
      //    for (size_t v_idx = 0; v_idx < num_para_v; ++v_idx) {
      //      float para_v = interval_v * v_idx;
      //      glm::vec3 pt = nurbs::SurfacePoint(srf2, para_u, para_v);
      //      surface.at(num_para_v * u_idx + v_idx) = pt;
      //    }
      //  }

      //  srf = srf2;
      //  ChangeOutData();
      //}
      //else {
      //  std::cerr << "Parameters(degree and control points) are wrong!" << std::endl;
      //}
    //}

    // Set draw mode
    {
      static int draw_mode = 2;
      ImGui::RadioButton("Fill", &draw_mode, 2); ImGui::SameLine();
      ImGui::RadioButton("Wireframe", &draw_mode, 1); ImGui::SameLine();
      ImGui::RadioButton("Point", &draw_mode, 0);
      ChangeDrawMode(draw_mode);
    }
  }

  if (ImGui::CollapsingHeader("Curve")) {
    auto &cp = curve_primitive.control_points;
    auto &knots = curve_primitive.knots;
    auto &degree = curve_primitive.degree;
    static bool curve_cp_changed = true, curve_degree_changed = true;

    // The number of control points
    {
      static int num_curve_con_points = 4;
      ImGui::PushFont(GetBoldFont());
      ImGui::Text("The number of C.P.");
      ImGui::PopFont(); ImGui::SameLine();
      ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() * 0.4f);
      curve_cp_changed |= ImGui::DragInt("##curve-num-C.P.", &num_curve_con_points, 0.1, 2, 20, "%d");
      ImGui::PopItemWidth();
      if (curve_cp_changed) cp.resize(num_curve_con_points);
    }

    // Customize control points
    if (ImGui::TreeNode("Customize control points##curve")) {
      ImGui::BeginGroup();
      ImGui::BeginChild("##curve-cp-list", ImVec2(0, 150), true);
      for (size_t u_idx = 0; u_idx < cp.size(); u_idx++) {
        std::stringstream idx;
        idx << u_idx;
        std::string dummy_name("##dummy-curve-cp");
        dummy_name.append(idx.str());
        ImGui::Text("C.P.(%d)", u_idx);
        ImGui::SameLine();
        curve_cp_changed |= ImGui::InputFloat3(dummy_name.c_str(), cp.at(u_idx).data.data);
      }
      ImGui::EndChild();
      ImGui::EndGroup();

      ImGui::TreePop();
    }

    // Parameterization
    static std::vector<float> parameters;
    {
      static const char* parameterization_methods[] = { "Equally spaced", "Chord length", "Centripetal" };
      static int para_current = 1;
      curve_cp_changed |= ImGui::Combo("Parameterization method",
          &para_current, parameterization_methods, IM_ARRAYSIZE(parameterization_methods));
      static const std::function<
          float(glm::vec3 &delta,
                float total_length,
                int parameters)> parameterizations[] = {
        [](glm::vec3 &delta, float total_length, int parameters) -> float {
          return 1.0f / (parameters - 1);
        },
        [](glm::vec3 &delta, float total_length, int parameters) -> float {
          float norm = glm::length(delta);
          return norm / total_length;
        },
        [](glm::vec3 &delta, float total_length, int parameters) -> float {
          float norm = glm::length(delta);
          return glm::sqrt(norm) / total_length;
        }
      };
      if (curve_cp_changed) {
        parameters.resize(cp.size());

        float total_chord_length = 0;
        glm::vec3 delta;
        for (int idx = 1; idx < cp.size(); idx++) {
          delta = cp.at(idx) - cp.at(idx - 1);
          total_chord_length += glm::length(delta);
        }

        parameters.at(0) = 0.0f;
        for (int idx = 1; idx < cp.size() - 1; idx++) {
          delta = cp.at(idx) - cp.at(idx - 1);
          parameters.at(idx) = parameters.at(idx - 1) +
            parameterizations[para_current](delta, total_chord_length, cp.size());
        }
        parameters.at(cp.size() - 1) = 1.0f;
      }
    }

    // Degrees
    {
      static int curve_degree = 2;
      ImVec4 color(ImGui::GetStyleColorVec4(ImGuiCol_Text));
      if (degree >= cp.size())
        color = ImGui::GetStyleColorVec4(ImGuiCol_PlotLinesHovered);
      ImGui::PushFont(GetBoldFont());
      ImGui::TextColored(color, "Degree");
      ImGui::PopFont(); ImGui::SameLine();
      ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() * 0.4f);
      curve_degree_changed |= ImGui::DragInt("##curve-degree", &curve_degree, 0.1, 1, cp.size() - 1, "%d");
      if (curve_degree_changed) degree = curve_degree;
      ImGui::PopItemWidth();
    }

    // Make knots
    {
      static const char* knot_methods[] = { "Equally spaced", "Averaging" };
      static int knot_current = 1;
      curve_degree_changed |= ImGui::Combo("Knot vetor method",
          &knot_current, knot_methods, IM_ARRAYSIZE(knot_methods));
      static const std::function<
          float(std::vector<float> &parameters,
                int degree,
                int idx)> knot_vector_methods[] = {
        [](std::vector<float> &parameters, int degree, int idx) -> float {
          return static_cast<float>(idx - degree) / (parameters.size() - degree);
        },
        [](std::vector<float> &parameters, int degree, int idx) -> float {
          float knot_value = 0;
          for (int ii = idx - degree; ii <= idx - 1; ii++) {
              knot_value += parameters.at(ii);
          }
          knot_value /= degree;
          return knot_value;
        }
      };
      if (curve_cp_changed || curve_degree_changed) {
        knots.resize(cp.size() + degree + 1);
        for (size_t u_idx = degree + 1; u_idx < cp.size(); u_idx++) {
          knots.at(u_idx) = knot_vector_methods[knot_current](parameters, degree, u_idx);
        }
        for (size_t u_idx = 0; u_idx <= degree; u_idx++) {
          knots.at(u_idx) = 0.0f;
          knots.at(knots.size() - 1 - u_idx) = 1.0f;
        }
        curve_cp_changed = curve_degree_changed = false;
      }
    }

    // Customize knots
    if (ImGui::TreeNode("Customize knots")) {
      ImGui::BeginGroup();
      ImGui::BeginChild("##curve-knots-list", ImVec2(0, 150), true);
      for (size_t u_idx = 0; u_idx < knots.size(); u_idx++) {
        std::stringstream idx;
        idx << u_idx;
        std::string knot_name("##curve-knot");
        knot_name.append(idx.str());
        ImGui::Text("Knot(%d)", u_idx); ImGui::SameLine();
        ImGui::InputFloat(knot_name.c_str(), &knots.at(u_idx));
      }
      ImGui::EndChild();
      ImGui::EndGroup();

      ImGui::TreePop();
    }
    
    ImGui::Separator();
    ImGui::Checkbox("Tangent", &show_tangent); ImGui::SameLine();
    ImGui::Checkbox("Normal", &show_normal); ImGui::SameLine();
    ImGui::Checkbox("Binormal", &show_binormal);
    // Make curve
    if (ImGui::Button("Make curve")) {
      auto &weight = curve_primitive.weights;
      weight.resize(curve_primitive.control_points.size(), 1);

      if (nurbs::CurveIsValid(curve_primitive)) {
        static float interval = 0.01f;
        num_curve_para = 1 / interval + 1;
        curve_points.resize(num_curve_para);
        curve_tangents.resize(num_curve_para);
        curve_normals.resize(num_curve_para);
        curve_binormals.resize(num_curve_para);

        for (size_t u_idx = 0; u_idx < curve_points.size(); u_idx++) {
          float para = interval * u_idx;
          glm::vec3 pt = nurbs::CurvePoint(curve_primitive, para);
          curve_points.at(u_idx) = pt;

          glm::vec3 c1 = nurbs::CurveDerivatives(curve_primitive, 1, para).at(1);
          curve_tangents.at(u_idx) = glm::normalize(c1);

          glm::vec3 c2 = nurbs::CurveDerivatives(curve_primitive, 2, para).at(2);
          glm::vec3 binormal = glm::cross(c1, c2);
          curve_binormals.at(u_idx) = glm::normalize(binormal);

          float nomi = glm::length(binormal);
          float denomi = glm::pow(glm::length(c1), 3);
          float curvature = nomi / denomi;
          glm::vec3 normal = curvature * glm::normalize(glm::cross(binormal, c1));
          curve_normals.at(u_idx) = normal;
        }

        ChangeOutData2();
      }
    }
    ImGui::SameLine();

    // Clear curve
    if (ImGui::Button("Clear curve")) {
      GetVertices2()  = std::vector<glm::vec3>();
      GetColors2()    = std::vector<glm::vec3>();
      GetIndices2()   = std::vector<GLuint>();

      BindBuffers2();
    }
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
        float degreesPerPixel = 0.01f;
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

void TestApp::MakeSurface(
    unsigned int degree_u,
    unsigned int degree_v,
    std::vector<float> &knots_u,
    std::vector<float> &knots_v,
    nurbs::array2<glm::vec3> &control_points,
    nurbs::array2<float> &weigths) {
  surface_primitive.degree_u = degree_u;
  surface_primitive.degree_v = degree_v;
  surface_primitive.knots_u = knots_u;
  surface_primitive.knots_v = knots_v;
  surface_primitive.control_points = control_points;
  surface_primitive.weights = weigths;

  /*
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
  */
}

} // inline namespace opengl3

} // namespace vktuto