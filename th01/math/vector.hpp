#pragma once
// Different parameter lists and calling conventions compared to later games!

#ifdef TH01_NATIVE_ORB_PHYSICS
#include "native/physics_types.hpp"
void vector2(int16_t& ret_x,int16_t& ret_y,int16_t length,unsigned char angle);
void vector2_between(int16_t x1,int16_t y1,int16_t x2,int16_t y2,int16_t& ret_x,int16_t& ret_y,int16_t length);
#else

void vector2(int &ret_x, int &ret_y, int length, unsigned char angle);

void vector2_between(
	int x1, int y1, int x2, int y2, int& ret_x, int &ret_y, int length
);
#endif
