#include "stage_tick.hpp"
#include "stage_pellet_binding.hpp"
#include "obstacle_backend.hpp"
#include "th01/main/stage/stages.hpp"
#include <utility>
namespace {
std::function<void(unsigned)> wait_callback;
void frame_delay(unsigned frames) {
	if(!wait_callback) throw std::runtime_error("Native stage timing is not bound");
	wait_callback(frames);
}
// Original counter is unused and intentionally remains private to this unit.
int orb_frames_outside_portal=0;
constexpr double COEFFICIENT_OF_RESTITUTION=0.78;
}
namespace th01_native {
StageTimingBinding::StageTimingBinding(std::function<void(unsigned)> wait):previous(std::move(wait_callback)) {
	wait_callback=std::move(wait);
}
StageTimingBinding::~StageTimingBinding() {wait_callback=std::move(previous);}
}
// Shared with the DOS build: no copied or reordered gameplay frame body.
#include "th01/main/player/orb_stage.inc"
