#ifdef TH01_NATIVE_ORB_PHYSICS
#include "native/math_backend.hpp"
#else
#include "libs/master.lib/master.hpp"
#include "pc98.h"
#endif

inline pixel_t wave_x(pixel_t amp, int t) {
	return ((static_cast<long>(amp) * Sin8(t)) / 0x100);
}
