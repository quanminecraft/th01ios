#pragma once
#include "player_backend.hpp"
#include "th01/main/player/player.hpp"
#include "th01/main/bullet/laser_s.hpp"
#include "math_backend.hpp"
using dots16_t=std::uint16_t;
constexpr int PRESHIFT=8,BYTE_MASK=7,BYTE_DOTS=8;
std::uint16_t native_laser_mask(unsigned index);
void native_laser_put(screen_x_t left,vram_y_t top,dots16_t mask,vc2 color);
namespace th01_native {
void update_shootout_lasers();
}
