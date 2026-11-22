#pragma once
#include "card_backend.hpp"
#include "pellet_backend.hpp"
#include <utility>

namespace th01_native {
// Keep this binding alive for the stage session. Creation happens inside
// the original card/turret call, before another actor can consume RNG.
class StagePelletBinding {
	CardEvents& events;
	decltype(CardEvents::pellet_dispatch) previous;
public:
	explicit StagePelletBinding(CardEvents& target):events(target),previous(std::move(target.pellet_dispatch)) {
		events.pellet_dispatch=[](screen_x_t left,screen_y_t top,pellet_group_t group,subpixel_t speed) {
			Pellets.add_group(left,top,group,speed);
		};
	}
	~StagePelletBinding() {events.pellet_dispatch=std::move(previous);}
	StagePelletBinding(const StagePelletBinding&)=delete;
	StagePelletBinding& operator=(const StagePelletBinding&)=delete;
};
}
