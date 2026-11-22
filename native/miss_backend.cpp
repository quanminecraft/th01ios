#include "miss_backend.hpp"
namespace {
th01_native::MissBackend* active=nullptr;
th01_native::MissBackend& backend() {if(!active) throw std::runtime_error("Native miss backend is not bound");return *active;}
void frame_delay(unsigned frames) {backend().delay(frames);}
inline screen_y_t player_48_left_for(screen_x_t regular_left) {return (regular_left-((48-PLAYER_W)/2));}
inline screen_y_t player_48x48_top() {return (PLAYFIELD_BOTTOM-48);}
inline void orb_put_default() {ptn_put_8(orb_cur_left,orb_cur_top,PTN_ORB);}
}
namespace th01_native {
MissBackendBinding::MissBackendBinding(MissBackend& value):previous(active) {
	if(!value.delay) throw std::invalid_argument("Miss timing integration is incomplete");
	active=&value;
}
MissBackendBinding::~MissBackendBinding() {active=previous;}
bool resume_after_miss() {
	if(!player_is_hit && !stage_cleared) throw std::logic_error("Miss continuation requires a completed life loop");
	test_damage=false;bomb_frame=200;
	if(stage_cleared || native_extend_lives()<=0) return false;
	auto& miss=backend();
	mdrv2_se_play(5);--native_extend_saved_lives();--native_extend_lives();
	player_miss_animate_and_update();player_is_hit=false;
	++miss.bgm_reload_counter;player_invincibility_time=PLAYER_MISS_INVINCIBILITY_FRAMES;
	return true;
}
}
#define rem_lives native_extend_lives()
void native_miss_delay(unsigned frames) {backend().delay(frames);}
#include "th01/main/player/player_miss.inc"
