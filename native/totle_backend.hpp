#pragma once
#include "totle_transition.hpp"
#include "extend_backend.hpp"
#include "input_backend.hpp"
#include "bonus_values.hpp"
#include "th01/main/stage/stages.hpp"
namespace th01_native {
struct TotleBackend {
	Framebuffer& framebuffer;
	const SpriteSet& clear_screen;
	const SpriteSet& numbers;
	std::function<InputKeys()> poll;
	std::function<void(unsigned)> wait,delay_ms;
	bool sloppy=false,loaded=false;
 std::function<void()> automatic_result;
};
class TotleBinding {
	TotleBackend* previous;
public:
	explicit TotleBinding(TotleBackend& backend);
	~TotleBinding();
	TotleBinding(const TotleBinding&)=delete;
	TotleBinding& operator=(const TotleBinding&)=delete;
};
}
void totle_animate(int stage);
extern std::uint32_t score_bonus;
