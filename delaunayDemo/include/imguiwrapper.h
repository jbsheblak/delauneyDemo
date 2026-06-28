
#pragma once

#include "imgui.h"

// need to hook this into the windows proc
#include <windows.h>
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// =================================================================
// NRenderer
// =================================================================

namespace imguiwrapper
{
   bool initialize(HWND hWnd);
   void shutdown();
   void new_frame();
   void render();
} // namespace imguiwrapper