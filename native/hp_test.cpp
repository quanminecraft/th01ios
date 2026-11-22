#include "hp_backend.hpp"
#include "boss_hit_backend.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main(int argc,char** argv) {
	try {
		auto fb=std::make_unique<Framebuffer>();for(unsigned i=0;i<16;++i) fb->palette[i]=0xff000000u|i*0x111111u;
		SpriteSet sprites;for(unsigned i=0;i<34;++i) sprites.images.push_back({32,32,Bytes(1024,2),Bytes(1024,1)});
		ShotBackend shots{*fb,sprites,{}};ShotBackendBinding shot_binding(shots);PlayerBackend player{shots,sprites,sprites,sprites,{},{},{}};PlayerBackendBinding player_binding(player);
		std::int16_t lives=4;HudIconsBackend hud{*fb,lives,{}};HudIconsBinding hud_binding(hud);
		fb->clear(0,3);fb->clear(1,3);hud_hp_first_redwhite=2;hud_hp_first_white=6;bool done=false;
		for(int i=0;i<8;++i) {hud_hp_increment_render(done,8,i);require(done==(i==7),"Original increment terminates on exact last point");}
		for(unsigned page=0;page<2;++page) {
			require(fb->pixel(page,128,48)==10 && fb->pixel(page,144,48)==10 && fb->pixel(page,145,48)==7 && fb->pixel(page,176,48)==7,"HP red, striped and white sections match GRCG tile");
			require(fb->pixel(page,135,48)==3 && fb->pixel(page,128,62)==10 && fb->pixel(page,128,63)==3,"Seven opaque columns and 15-row height preserve background gaps");
		}
		hud_hp_decrement(7);require(fb->pixel(0,184,48)==3 && fb->pixel(0,176,48)==7 && fb->pixel(1,184,48)==3,"Odd decrement restores pair then redraws surviving point on both pages");
		hud_hp_decrement(6);require(fb->pixel(0,176,48)==3 && fb->pixel(0,168,48)==10,"Even decrement restores paired background without extra redraw");
		CardEvents events;CardBackendBinding card_binding(events);BombBackend bomb{*fb,sprites,sprites,[] {}};BombBackendBinding bomb_binding(bomb,player);
		std::uint32_t frame=1;BossHitBackend boss{frame,lives,hud_hp_decrement,[] {}};BossHitBinding boss_binding(boss);
		boss_hit_int_t invframe=0,hp=6;bool16 invincible=false;const vc_t colors[]={2};test_damage=false;bomb_damaging=false;score=0;orb_velocity_x=OVX_0;orb_cur_left=100;
		boss_hit_update_and_render(invframe,invincible,hp,colors,1,1000,true);
		require(hp==5 && fb->pixel(0,168,48)==3 && fb->pixel(0,160,48)==10 && fb->pixel(1,168,48)==3,"Original boss hit callback updates actual HP background/stripe rendering");
		if(argc==2) dump_bmp(argv[1],*fb);
		fb->clear(0,5);fb->clear(1,5);page_access(0);hud_hp_rerender(8);hud_hp_decrement(7);
		require(fb->pixel(0,184,48)==5 && fb->pixel(1,184,48)==5,"Rerender captures changed boss background");
		done=true;auto before=fb->pages;hud_hp_increment_render(done,8,9);require(fb->pages==before,"Completed HP animation ignores further calls");
		std::cout<<"Original boss HP increment, colors, paired restore, rerender and hit integration passed\n";
	} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
