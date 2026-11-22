#pragma once
#include "shot_backend.hpp"
#include "mima_square_patterns.hpp"
using vram_y_t=std::int16_t;using vc2=std::uint8_t;
void graph_r_line(screen_x_t left,vram_y_t top,screen_x_t right,vram_y_t bottom,vc2 color);
void native_graph_line_unput(screen_x_t left,vram_y_t top,screen_x_t right,vram_y_t bottom);
void graph_r_line_patterned(screen_x_t left,vram_y_t top,screen_x_t right,vram_y_t bottom,vc2 color,std::uint16_t pattern);
namespace th01_native {
void render_mima_square(bool restore,const std::array<MimaPoint,4>& points);
}
