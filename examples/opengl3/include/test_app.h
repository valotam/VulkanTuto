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
}; // class TestApp

} // inline namespace opengl3

} // namespace vktuto