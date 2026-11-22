#pragma once
#include "physics_types.hpp"
extern const std::int16_t SinTable8[256],CosTable8[256];
extern const std::uint8_t AtanTable8[256];
#define Sin8(t) SinTable8[(t)&255]
#define Cos8(t) CosTable8[(t)&255]
std::int16_t iatan2(std::int16_t y,std::int16_t x);
