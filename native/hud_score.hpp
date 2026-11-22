#pragma once
#include "hud_icons.hpp"
#include "card_backend.hpp"
#include <utility>
extern bool fwnum_force_rerender;
extern unsigned char hud_cardcombo_max;
void native_hud_score_init(bool16 first_run);
void native_hud_score_render();
namespace th01_native {
class HudScoreBinding {
	CardEvents& events;
	std::function<void()> previous;
public:
	explicit HudScoreBinding(CardEvents& value):events(value),previous(std::move(value.hud_dispatch)) {events.hud_dispatch=native_hud_score_render;}
	~HudScoreBinding() {events.hud_dispatch=std::move(previous);}
	HudScoreBinding(const HudScoreBinding&)=delete;
	HudScoreBinding& operator=(const HudScoreBinding&)=delete;
};
}
