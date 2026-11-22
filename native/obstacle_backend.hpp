#pragma once
#include "card_backend.hpp"
#include "th01/formats/stagedat.hpp"
#include <cstdlib>

namespace th01_native {
struct ObstacleState {
	union Frame {
		std::int16_t since_collision;
		std::int16_t fire_cycle;
		std::int16_t v;
	};
	std::int16_t count=0;
	std::vector<screen_x_t> left;
	std::vector<screen_y_t> top;
	std::vector<obstacle_type_t> type;
	std::vector<Frame> frame;
};
// Initialize storage; the caller invokes the original reset/update routine
// with the orb in the intended startup position, as in original stage setup.
void load_obstacles(const StageLayout& layout);
}
extern th01_native::ObstacleState obstacles;
void portal_enter_update_and_render_or_reset(int slot,bool16 reset);
void turret_fire_update_and_render_or_reset(int slot,bool16 reset);
void obstacles_update_and_render(bool16 reset);
#define portal_enter_update_and_render(slot) portal_enter_update_and_render_or_reset(slot,false)
#define turret_fire_update_and_render(slot) turret_fire_update_and_render_or_reset(slot,false)
#define portals_reset() portal_enter_update_and_render_or_reset(i,reset)
#define turrets_reset() turret_fire_update_and_render_or_reset(i,reset)
#define stageobj_put_8(objects,i,id) ptn_put_8(objects.left[i],objects.top[i],id)
#define stageobj_sloppy_unput_16(objects,i) ptn_sloppy_unput_16(objects.left[i],objects.top[i])
