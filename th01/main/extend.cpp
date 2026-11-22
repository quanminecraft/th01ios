#ifdef TH01_NATIVE_ORB_PHYSICS
#include "native/extend_backend.hpp"
#define rem_lives native_extend_lives()
#define resident_lives native_extend_saved_lives()
#else
#include "th01/main/extend.hpp"
#define resident_lives resident->rem_lives
#endif

#ifdef TH01_NATIVE_ORB_PHYSICS
std::int16_t extend_next = 1;
#else
int extend_next = 1;
#endif

void score_extend_update_and_render(void)
{
	if(score >= (extend_next * SCORE_PER_EXTEND)) {
#ifdef TH01_NATIVE_ORB_PHYSICS
        native_extend_threshold();
#endif
		if(rem_lives < LIVES_MAX) {
			rem_lives++;
			resident_lives = rem_lives;
			hud_lives_put(rem_lives - 1);
			mdrv2_se_play(15);
			pellet_speed_raise(pellet_speed_t(1));
		}
		extend_next++;
	}
}
#ifdef TH01_NATIVE_ORB_PHYSICS
#undef rem_lives
#endif
#undef resident_lives

