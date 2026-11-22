#pragma once
#include "physics_types.hpp"
#include <array>
using vram_y_t=std::int16_t;
constexpr int RES_X=640;
struct NativeShape8 {
	std::array<std::uint8_t,8> dots{};
	constexpr int w() const {return 8;}
	constexpr int h() const {return 8;}
	std::uint8_t operator[](unsigned row) const {return dots.at(row);}
};
