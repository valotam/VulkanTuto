#pragma once

#include "imgui.h"

namespace vktuto {

inline namespace utility {

// Demonstrate creating a simple console window, with scrolling, filtering,
// completion and history. For the console example, here we are using a more
// C++ like approach of declaring a class to hold the data and the functions.
struct ConsoleApp
{
  char                    InputBuf[256];
  ImVector<char *>        Items;
  bool                    ScrollToBottom;
  ImVector<char *>        History;
  // -1: new line, 0..History.Size-1 browsing history.
  int                     HistoryPos;
  ImVector<const char *>  Commands;

  ConsoleApp();
  ~ConsoleApp();

  // Portable helpers
  static int    Stricmp(const char *str1, const char *str2);
  static int    Strnicmp(const char *str1, const char *str2, int n);
  static char * Strdup(const char *str);
  static void   Strtrim(char *str);

  void          ClearLog();

  void          AddLog(const char* fmt, ...) IM_FMTARGS(2);

  void          Draw();

  void          ExecCommand(const char *command_line);

  // In C++11 you are better off using lambdas for this sort of forwarding
  // callbacks
  static int    TextEditCallbackStub(ImGuiInputTextCallbackData *data);

  int           TextEditCallback(ImGuiInputTextCallbackData *data);
}; // struct ConsoleApp


} // inline namespace utility

} // name vktuto