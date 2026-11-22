#pragma once
#include "input_backend.hpp"
#include <functional>
namespace th01_native {
struct RegularFrameState {
	std::uint32_t frame_rand=0,frame_since_start_of_binary=0;
	std::int16_t lives=4;
};
// Remaining systems are explicit required integrations, never silent stubs.
struct RegularFrameHooks {
	std::function<void()> timer,shootout_lasers,score_extend;
	std::function<void(bool16)> invincibility;
	std::function<bool()> pause_menu;
	std::function<void()> boss_update;
};
enum class FrameResult { running,hit,stage_cleared,quit };
FrameResult regular_frame(RegularFrameState& state,int stage_id,const InputKeys& keys,const RegularFrameHooks& hooks);
}
