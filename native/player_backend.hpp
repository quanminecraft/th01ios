#pragma once
#include "shot_backend.hpp"
#include "th01/hardware/input.hpp"
#include <functional>

namespace th01_native {
struct PlayerBackend {
	ShotBackend& shots;
	const SpriteSet& miko;
	const SpriteSet& action48;
	const SpriteSet& action32;
	// Bomb visuals, HUD and pellet speed belong to their respective systems.
	// Missing handlers fail explicitly when used; there are no silent stubs.
	std::function<bool16(int)> bomb_update;
	std::function<void(int)> bombs_hud;
	std::function<void(float)> raise_pellet_speed;
};
class PlayerBackendBinding {
	PlayerBackend* previous;
public:
	explicit PlayerBackendBinding(PlayerBackend& backend);
	~PlayerBackendBinding();
	PlayerBackendBinding(const PlayerBackendBinding&)=delete;
	PlayerBackendBinding& operator=(const PlayerBackendBinding&)=delete;
};
class PlayerAnim {
	bool short_image;
public:
	explicit PlayerAnim(bool short_image):short_image(short_image) {}
	void unput_8(screen_x_t left,screen_y_t top,int image) const;
	void put_0_8(screen_x_t left,screen_y_t top,int image) const;
	void unput_and_put_overlapped_8(screen_x_t put_left,screen_y_t put_top,screen_x_t unput_left,screen_y_t unput_top,int put_image,int unput_image) const;
};
}
extern th01_native::PlayerAnim player_48x48,player_48x32;
extern std::uint8_t input_lr,rank;
extern bool input_shot,input_strike,input_bomb,bomb_damaging;
extern std::int8_t rem_bombs;
extern std::uint32_t bomb_frame;
void ptn_put_8(screen_x_t left,screen_y_t top,int id);
void ptn_unput_8(screen_x_t left,screen_y_t top,int id);
bool16 bomb_update_and_render(int frame);
void hud_bombs_put(int previous);
void pellet_speed_raise(float amount);
