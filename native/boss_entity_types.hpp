#pragma once
#include "shot_backend.hpp"
using vram_y_t=std::int16_t;
constexpr int SINGYOKU_W=96,SINGYOKU_H=96,PF_FN_LEN=13;
#ifndef BYTE_DOTS
#define BYTE_DOTS 8
#endif
template<class X,class Y> struct LRTB {X left{},right{};Y top{},bottom{};};
struct BOS {std::uint8_t vram_w=0,bos_image_count=0;pixel_t h=0;};
