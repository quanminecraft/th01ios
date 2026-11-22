#ifdef TH01_NATIVE_ORB_PHYSICS
#include "native/pellet_speed_binding.hpp"
#define resident_pellet_speed native_pellet_speed
#else
#define resident_pellet_speed resident->pellet_speed
#endif
void pellet_speed_lower(pellet_speed_t max, pellet_speed_t negative_delta)
{
	if(resident_pellet_speed > max) {
		resident_pellet_speed = max;
	} else {
		resident_pellet_speed += negative_delta;
	}
	if(resident_pellet_speed < PELLET_SPEED_LOWER_MIN) {
		resident_pellet_speed = PELLET_SPEED_LOWER_MIN;
	}
}

void pellet_speed_raise(pellet_speed_t delta)
{
	resident_pellet_speed += delta;
	if(resident_pellet_speed > PELLET_SPEED_RAISE_MAX) {
		resident_pellet_speed = PELLET_SPEED_RAISE_MAX;
	}
}
