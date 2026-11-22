#pragma once
#include "player_backend.hpp"
#include <map>
namespace th01_native {
struct HudIconsBackend {
	Framebuffer& framebuffer;
	std::int16_t& lives;
	std::map<std::pair<int,int>,std::array<std::uint8_t,256>> backgrounds;
	std::uint32_t hiscore=0;
 bool hide_lives=false;
 std::function<void()> challenge_init;
};
class HudIconsBinding {
	HudIconsBackend* previous;
public:
	explicit HudIconsBinding(HudIconsBackend& backend);
	~HudIconsBinding();
	HudIconsBinding(const HudIconsBinding&)=delete;
	HudIconsBinding& operator=(const HudIconsBinding&)=delete;
};
}
void native_hud_lives_init();
void native_hud_bombs_init();
void native_hud_lives_render(int previous);
void native_hud_bombs_render(int previous);
void native_hud_bg_snap(screen_x_t left,screen_y_t top,int id,int quarter);
void native_hud_bg_restore(screen_x_t left,screen_y_t top,int id,int quarter);
void native_hud_copy_row(screen_x_t left,screen_y_t top,pixel_t width);
std::uint32_t& native_hud_hiscore();
