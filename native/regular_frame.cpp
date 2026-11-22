#include "regular_frame.hpp"
#include "item_runtime.hpp"
#include "pellet_backend.hpp"
#include "stage_tick.hpp"
#include "th01/main/stage/stages.hpp"
namespace th01_native {
FrameResult regular_frame(RegularFrameState& state,int stage_id,const InputKeys& keys,const RegularFrameHooks& hooks) {
	if(stage_id<0 || stage_id>=STAGE_COUNT || (stage_is_boss(stage_id) && !hooks.boss_update))
		throw std::invalid_argument("Gameplay frame requires a valid stage and an explicit boss integration for boss stages");
	if(!hooks.timer || !hooks.shootout_lasers || !hooks.score_extend || !hooks.invincibility || !hooks.pause_menu)
		throw std::invalid_argument("Native gameplay frame integrations are incomplete");
	if(stage_cleared) return FrameResult::stage_cleared;
	if(player_is_hit) return FrameResult::hit;
	auto& frame_rand=state.frame_rand;
	auto& frame_since_start_of_binary=state.frame_since_start_of_binary;
	auto& rem_lives=state.lives;
	std::int16_t pellet_speed_raise_cycle;
	set_input_keys(keys);
	#include "th01/main/gameplay_prefix.inc"
	if((frame_rand&3)==0) hooks.timer();
	if(mode_test) {
		if(input_ok) test_damage=true;
		if(input_down) {player_is_hit=true;rem_lives=0;}
	}
	hooks.invincibility(player_invincible);
	if(stage_is_boss(stage_id)) hooks.boss_update();
	hooks.shootout_lasers();
	orb_and_pellets_and_stage_unput_update_render__vsync_wait(stage_id);
	if(paused && hooks.pause_menu()) return FrameResult::quit;
	hooks.score_extend();
	if(pellet_destroy_score_delta) {
		score+=pellet_destroy_score_delta;hud_score_and_cardcombo_render();pellet_destroy_score_delta=0;
	}
	if(stage_cleared) return FrameResult::stage_cleared;
	return player_is_hit ? FrameResult::hit : FrameResult::running;
}
}
