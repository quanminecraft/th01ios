#include "extend_backend.hpp"
#include "pellet_backend.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main() {
	try {
		static_assert(sizeof(pellet_speed_t)==2 && sizeof(extend_next)==2);
		auto fb=std::make_unique<Framebuffer>();SpriteSet sprites;
		ShotBackend shots{*fb,sprites,{}};ShotBackendBinding shot_binding(shots);
		PlayerBackend player{shots,sprites,sprites,sprites,{},{},{}};PlayerBackendBinding player_binding(player);PelletSpeedBinding speed_binding(player);
		native_pellet_speed=0;pellet_speed_raise(0.025f);require(native_pellet_speed==1,"Float speed request converts to original units of 1/40");
		pellet_speed_raise(0.5f);require(native_pellet_speed==20,"Raise clamps to original maximum");
		pellet_speed_lower(0.0f,-0.05f);require(native_pellet_speed==0,"Lower above maximum clamps without adding delta");
		pellet_speed_lower(0.0f,-0.05f);require(native_pellet_speed==-2,"Lower at maximum applies negative delta");
		for(int i=0;i<20;++i) pellet_speed_lower(0.0f,-0.05f);
		require(native_pellet_speed==-15,"Lower minimum is negative 0.375 in original units");
		std::int16_t lives=4;std::int8_t saved=4;std::vector<int> hud;
		ExtendBackend backend{lives,saved,[&](int previous) {hud.push_back(previous);}};ExtendBackendBinding binding(backend);
		extend_next=1;score=399999;native_pellet_speed=0;score_extend_update_and_render();
		require(lives==4 && extend_next==1 && hud.empty(),"No life below 400000-point threshold");
		score=400000;score_extend_update_and_render();
		require(lives==5 && saved==5 && extend_next==2 && hud==std::vector<int>({4}) && shots.sound_commands==std::vector<unsigned>({15}) && native_pellet_speed==1,"Extend updates live/persisted lives, HUD, SE and integer pellet speed");
		rank=RANK_EASY;Pellets.native_reset_all();Pellets.add_group(100,100,PG_1,56);
		require(Pellets.native_at(0).velocity.y.v==57,"Extend speed modifier affects actual next projectile");
		score=2400000;score_extend_update_and_render();
		require(lives==6 && extend_next==3 && native_pellet_speed==2,"Only one threshold handled per frame even after large score jump");
		for(int i=0;i<4;++i) score_extend_update_and_render();
		require(lives==6 && saved==6 && extend_next==7 && hud.size()==2 && shots.sound_commands.size()==2 && native_pellet_speed==2,"Life cap consumes thresholds without HUD, SE or speed increases");
		lives=5;score_extend_update_and_render();require(lives==5,"Consumed capped thresholds do not award a delayed life");
		std::cout << "Original pellet speed scaling/clamps and score/life extend integration passed\n";
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}
