#pragma once
#include "player_backend.hpp"
#include "th01/main/bullet/pellet_s.hpp"
#include <utility>
extern std::int16_t native_pellet_speed;
namespace th01_native {
class PelletSpeedBinding {
	PlayerBackend& player;
	decltype(PlayerBackend::raise_pellet_speed) previous;
public:
	explicit PelletSpeedBinding(PlayerBackend& value):player(value),previous(std::move(value.raise_pellet_speed)) {
		player.raise_pellet_speed=[](float delta) {pellet_speed_raise(to_pellet_speed(delta));};
	}
	~PelletSpeedBinding() {player.raise_pellet_speed=std::move(previous);}
	PelletSpeedBinding(const PelletSpeedBinding&)=delete;
	PelletSpeedBinding& operator=(const PelletSpeedBinding&)=delete;
};
}
