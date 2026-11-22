#include "timer_backend.hpp"
#include "pellet_speed_binding.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main() {
	try {
		static_assert(sizeof(stage_timer)==2);
		auto fb=std::make_unique<Framebuffer>();SpriteSet sprites;
		ShotBackend shots{*fb,sprites,{}};ShotBackendBinding shot_binding(shots);
		PlayerBackend player{shots,sprites,sprites,sprites,{},{},{}};PlayerBackendBinding player_binding(player);PelletSpeedBinding speed_binding(player);
		Bytes bytes(0x46800);for(unsigned c='0';c<='9';++c) bytes[0x1800+(0x29-0x21)*96*32+(c-0x20)*32]=0x80;
		FontRom font(std::move(bytes));CardRenderer renderer{*fb,sprites,font,{}};CardEvents events;events.renderer=&renderer;CardBackendBinding card_binding(events);
		unsigned animations=0;TimerBackend timer{*fb,{}, {},[&]() {++animations;}};TimerBackendBinding timer_binding(timer);
		timer_init_for(0,ROUTE_MAKAI);require(stage_timer==1000,"First-stage timer");
		timer_init_for(18,ROUTE_MAKAI);require(stage_timer==600,"Makai stage 19 timer");
		timer_init_for(18,ROUTE_JIGOKU);require(stage_timer==1600,"Jigoku route timer offset");
		timer_init_for(19,ROUTE_JIGOKU);require(stage_timer==8000,"Jigoku final-stage timer");
		timer_init_for(0,ROUTE_JIGOKU);fb->clear(0,3);fb->clear(1,5);page_access(0);timer_bg_snap_and_put();
		require(timer.captured[0] && timer.captured[1] && fb->pixel(0,512,0)==7 && fb->pixel(1,512,0)==7,"Timer snapshots and draws both pages");
		timer_tick_and_put();require(stage_timer==998 && current_render_page()==0,"Timer subtracts two and returns to page zero");
		require(fb->pixel(0,528,15)==3 && fb->pixel(1,528,15)==3,"Both timer quarters retain original same-source snapshot behavior");
		stage_timer=200;timer_tick_and_put();require(stage_timer==198 && fb->pixel(0,520,0)==2 && fb->pixel(1,520,0)==2,"Timer changes to red below 200");
		stage_timer=2;native_pellet_speed=0;timer_tick_and_put();
		require(stage_timer==0 && animations==1 && native_pellet_speed==2 && shots.sound_commands.back()==17,"Expiry raises pellet speed and triggers original hurry-up boundary/SE");
		Pellets.native_reset_all();native_random_seed=1;rank=RANK_LUNATIC;player_left=304;
		for(int i=0;i<4;++i) timer_tick_and_put();require(Pellets.native_alive_count()==0,"Initial hurry-up cadence waits four ticks");
		timer_tick_and_put();std::uint32_t seed=1;for(int i=0;i<8;++i) seed=seed*0x015a4e35u+1;
		require(Pellets.native_alive_count()==4 && native_random_seed==seed,"Lunatic hurry-up fires four actual bullets with original interleaved RNG");
		timer_extend_and_put();require(stage_timer==1000,"Extending expired timer grants 1000");
		timer_extend_and_put();require(stage_timer==1200,"Extending running timer grants 200");
		std::cout << "Original stage timer, route values, both-page rendering, expiry and hurry-up bullets passed\n";
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}
