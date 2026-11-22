#pragma once
#include "bomb_backend.hpp"
#include "card_backend.hpp"
#include "th01/main/player/bomb.hpp"
using boss_hit_int_t=std::int16_t;
using vc_t=std::uint8_t;
constexpr int BOSS_HIT_INVINCIBILITY_FRAMES=40;
namespace th01_native {
struct BossHitBackend {
	std::uint32_t& frame;
	std::int16_t& lives;
	std::function<void(int)> hp_decrement;
	std::function<void()> palette_show;
};
class BossHitBinding {
	BossHitBackend* previous;
public:
	explicit BossHitBinding(BossHitBackend& backend);
	~BossHitBinding();
	BossHitBinding(const BossHitBinding&)=delete;
	BossHitBinding& operator=(const BossHitBinding&)=delete;
};
}
std::uint32_t& native_boss_frame();
void native_boss_hp_decrement(int hp);
void boss_palette_show();
void boss_hit_update_and_render(boss_hit_int_t& invincibility_frame,bool16& is_invincible,boss_hit_int_t& hp,const vc_t colors[],unsigned char color_count,int hit_score,bool colliding_with_orb,screen_x_t shot_left=0,screen_y_t shot_top=0,pixel_t shot_w=0,pixel_t shot_h=0);
#define z_palette_flash_colors(colors,count,i) for(i=0;i<count;++i) {z_palette_set_show(colors[i],15,15,15);}
