#include "th01/math/polar.hpp"
#include "th01/math/vector.hpp"

void vector2_between(
#ifdef TH01_NATIVE_ORB_PHYSICS
	int16_t x1,int16_t y1,int16_t x2,int16_t y2,int16_t& ret_x,int16_t& ret_y,int16_t length
#else
	int x1, int y1, int x2, int y2, int& ret_x, int &ret_y, int length
#endif
)
{
	unsigned char angle = iatan2(y2 - y1, x2 - x1);
	ret_x = polar_x_fast(0, length, angle);
	ret_y = polar_y_fast(0, length, angle);
}

#ifdef TH01_NATIVE_ORB_PHYSICS
void vector2(int16_t& ret_x,int16_t& ret_y,int16_t length,unsigned char angle)
#else
void vector2(int &ret_x, int &ret_y, int length, unsigned char angle)
#endif
{
	ret_x = polar_x_fast(0, length, angle);
	ret_y = polar_y_fast(0, length, angle);
}
