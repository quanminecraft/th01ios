#pragma once
#include <functional>
#include "th01/main/player/orb.hpp"

namespace th01_native {
// Original gameplay calls the wait after moving the orb and before bullets.
// The host supplies timing/presentation; deterministic tests supply an observer.
class StageTimingBinding {
	std::function<void(unsigned)> previous;
public:
	explicit StageTimingBinding(std::function<void(unsigned)> wait);
	~StageTimingBinding();
	StageTimingBinding(const StageTimingBinding&)=delete;
	StageTimingBinding& operator=(const StageTimingBinding&)=delete;
};
}
