
#pragma once
#include <stdint.h>

// =================================================================
// NDemo
// =================================================================

namespace NDemo
{
   bool initialize();
   void shutdown();
   void simulate();
   void render();

   void add_mouse_click(int32_t const mouseX, int32_t const mouseY);
   void clear_mouse_clicks();
   void remove_last_triangle();
   void save_data();
   void read_data();
} // namespace NDemo