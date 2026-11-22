#include "boss_hit_backend.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool condition,const char* reason) {if(!condition) throw std::runtime_error(reason);}
int main() {
	try {
		auto fb=std::make_unique<Framebuffer>();SpriteSet sprites;
		for(unsigned i=0;i<34;++i) sprites.images.push_back({32,32,Bytes(1024,2),Bytes(1024,1)});
		ShotBackend shots{*fb,sprites,{}};ShotBackendBinding shot_binding(shots);
		PlayerBackend player{shots,sprites,sprites,sprites,{},{},{}};PlayerBackendBinding player_binding(player);
		BombBackend bomb{*fb,sprites,sprites,[] {}};BombBackendBinding bomb_binding(bomb,player);
		CardEvents events;CardBackendBinding card_binding(events);
		std::uint32_t frame=1;std::int16_t lives=4;std::vector<int> hp_updates;unsigned palette_resets=0;
		BossHitBackend backend{frame,lives,[&](int hp) {hp_updates.push_back(hp);},[&] {++palette_resets;}};BossHitBinding binding(backend);
		const vc_t flash[]={2,3};boss_hit_int_t invframe=0,hp=8;bool16 invincible=false;
		score=0;test_damage=false;bomb_damaging=false;orb_cur_left=100;orb_velocity_x=OVX_4_LEFT;
		for(unsigned i=0;i<8;++i) {Shots.moving[i]=false;Shots.decay_frame[i]=0;}
		auto hit=[&](bool orb) {boss_hit_update_and_render(invframe,invincible,hp,flash,2,1000,orb,100,100,32,32);};
		hit(true);require(hp==7 && invincible && invframe==0 && score==1000 && orb_velocity_x==OVX_4_RIGHT,"Orb hit damages, scores and reverses direction");
		require(hp_updates==std::vector<int>{7} && shots.sound_commands==std::vector<unsigned>{3} && palette_resets==1,"Hit HUD, SE and original immediate palette restore dispatch");
		invframe=1;hit(true);require(hp==7 && score==1000,"Invincibility blocks repeated orb damage");
		invframe=2;hit(false);require(fb->palette[2]==0xffffffffu && fb->palette[3]==0xffffffffu,"Invincibility flashes selected palette colors");
		invframe=40;hit(false);require(invincible && invframe==40,"Original exact-frame-40 boundary remains invincible");
		invframe=41;hit(false);require(!invincible && invframe==0,"Invincibility expires only beyond frame 40");
		auto resets=palette_resets;invframe=41;hit(false);require(palette_resets==resets+1 && invframe==0,"Original periodic palette reset also occurs without invincibility");
		Shots.add(110,110);hit(false);require(!Shots.moving[0] && Shots.decay_frame[0]==1 && hp==7,"Shot collision starts decay without damaging boss HP");
		test_damage=true;hit(false);require(hp==6 && !test_damage && score==1000,"Debug damage updates HP but does not grant hit score");
		bomb_damaging=true;frame=50;hit(false);require(hp==6,"Bomb cannot damage boss while reserve lives remain");
		lives=0;frame=49;hit(false);require(hp==6,"Last-life bomb damage is restricted to frame multiples of 50");
		frame=50;hit(false);require(hp==5 && invincible && score==2000,"Original last-life bomb damage grants normal hit effects");
		frame=100;hit(false);require(hp==4 && score==3000,"Bomb damage bypasses boss invincibility");
		bomb_damaging=false;invincible=false;orb_velocity_x=OVX_0;orb_cur_left=400;hit(true);require(orb_velocity_x==OVX_4_LEFT,"Stationary orb to right of center bounces left");
		std::cout<<"Original boss hit, shot decay, invincibility, palette and last-life bomb logic passed\n";
	} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
