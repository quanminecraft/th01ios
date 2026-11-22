#pragma once
#include "bomb_backend.hpp"
#include "th01/main/player/bomb.hpp"
#include "extend_backend.hpp"
#include "timer_backend.hpp"
void native_miss_delay(unsigned frames);
namespace th01_native {
struct MissBackend {
	std::function<void(unsigned)> delay;
	// Original counter: nonzero suppresses BGM reload on the next life.
	std::int16_t& bgm_reload_counter;
};
class MissBackendBinding {
	MissBackend* previous;
public:
	explicit MissBackendBinding(MissBackend& backend);
	~MissBackendBinding();
	MissBackendBinding(const MissBackendBinding&)=delete;
	MissBackendBinding& operator=(const MissBackendBinding&)=delete;
};
// False means the life loop must stop for stage completion or Game Over.
bool resume_after_miss();
}
