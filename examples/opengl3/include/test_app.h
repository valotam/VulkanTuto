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

 private:
  ImGuiWindowFlags window_flags_ = 0;
  bool show_main_window_ = true;

  ImGuiWindowFlags & SetupWindowFlags() const noexcept;

  void ShowMainWindow();
  
  void MenuTabs();
  void SplitView();
  
  void ControlsColumn();
  
  void TabsColumn();
  void CanvasTab();
  void ConsolTab();

  void CanvasContext();

  nurbs::RationalSurface3f MakeSurface(
      unsigned int degree_u,
      unsigned int degree_v,
      std::vector<float> knots_u,
      std::vector<float> knots_v,
      nurbs::array2<glm::vec3> control_points,
      nurbs::array2<float> weigths);

  unsigned int num_para_u = 0, num_para_v = 0;
  std::vector<glm::vec3> surface;
  nurbs::RationalSurface3f srf;

}; // class TestApp

} // inline namespace opengl3

} // namespace vktuto