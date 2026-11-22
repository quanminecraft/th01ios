#pragma once
#include <cstdint>
#ifndef near
#define near
#endif

// The first isolated gameplay component still uses the original global API.
// Preserve DOS-sized storage; host int/long must not define game data sizes.
using pixel_t=std::int16_t;
using upixel_t=std::uint16_t;
using pixel_length_8_t=std::uint8_t;
using pixel_delta_8_t=std::int8_t;
using screen_x_t=std::int16_t;
using screen_y_t=std::int16_t;
using bool16=std::int16_t;
using int16_t=std::int16_t;
constexpr screen_x_t PLAYFIELD_LEFT=0,PLAYFIELD_RIGHT=640;
constexpr screen_y_t PLAYFIELD_TOP=64,PLAYFIELD_BOTTOM=400;
constexpr screen_x_t PLAYFIELD_CENTER_X=320;
constexpr int ORB_CELS=4;

extern std::uint32_t native_random_seed;
int irand();
