#pragma once
#include "opengl3_base.h"
#include "nurbs/nurbs.h"

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

  virtual void ChangeOutData() override;
  virtual void ChangeOutData2() override;

 private:
  ImGuiWindowFlags window_flags_ = 0;
  bool show_main_window_ = true;

  glm::vec3 boundary[4] = {
    glm::vec3(-1, -1, 0), glm::vec3(-1, 1, 0), glm::vec3(1, -1, 0), glm::vec3(1, 1, 0)
  };
  int num_surface_con_point_u = 5, num_surface_con_point_v = 5;
  int degree_u = 2, degree_v = 2;
  unsigned int num_para_u = 0, num_para_v = 0;
  std::vector<glm::vec3> surface_points;
  nurbs::RationalSurface3f surface_primitive;
  nurbs::array2<glm::vec3> surface_control_points;

  bool show_tangent = false, show_normal = false, show_binormal = false;
  unsigned int num_curve_para = 0;
  std::vector<glm::vec3> curve_points;
  nurbs::RationalCurve3f curve_primitive;
  std::vector<glm::vec3> curve_tangents;
  std::vector<glm::vec3> curve_curvatures;
  std::vector<glm::vec3> curve_binormals;
  std::vector<glm::vec3> curve_normals;

  ImGuiWindowFlags & SetupWindowFlags() const noexcept;

  void ShowMainWindow();
  
  void MenuTabs();
  void SplitView();
  
  void ControlsColumn();
  
  void TabsColumn();
  void CanvasTab();
  void ConsolTab();

  void CanvasContext();

  void MakeSurface(
      unsigned int degree_u,
      unsigned int degree_v,
      std::vector<float> &knots_u,
      std::vector<float> &knots_v,
      nurbs::array2<glm::vec3> &control_points,
      nurbs::array2<float> &weigths);

}; // class TestApp

} // inline namespace opengl3

} // namespace vktuto