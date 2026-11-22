#pragma once
#include "hud_icons.hpp"
#include "th01/main/hud/hp.hpp"
using vram_y_t=std::int16_t;
#ifndef PTN_QUARTER_W
#define PTN_QUARTER_W 16
#endif
#define BYTE_DOTS 8
void native_hp_point_put(int point);
