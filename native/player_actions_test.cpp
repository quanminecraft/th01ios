#include "player_backend.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "th01/main/player/player.hpp"
#include "th01/main/player/orb.hpp"
#include "th01/main/player/shot.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* message) {if(!value) throw std::runtime_error(message);}
static SpriteSet synthetic(unsigned count,int w,int h) {
	SpriteSet set;
	for(unsigned i=0;i<count;++i) set.images.push_back({w,h,Bytes(w*h,std::uint8_t(i%14+1)),Bytes(w*h,1)});
	return set;
}
int main(int argc,char** argv) {
	try {
		if(argc>2) throw std::runtime_error("Usage: player_actions_test [ASSET_DIR]");
		SpriteSet stg,miko,action48,action32,background;
		if(argc==2) {
			auto archive=load_archive(argv[1]);
			stg=read_sprites(asset(archive,"STG.PTN"));miko=read_sprites(asset(archive,"MIKO.PTN"));
			action48=read_sprites(asset(archive,"MIKO_AC.BOS"));action32=read_sprites(asset(archive,"MIKO_AC2.BOS"));
			background=read_grp(read_file(std::filesystem::path(argv[1])/"ST0.GRP"));
		} else {stg=synthetic(34,32,32);miko=synthetic(28,32,32);action48=synthetic(28,48,48);action32=synthetic(6,48,32);}
		auto fb=std::make_unique<Framebuffer>();
		if(argc==2) {fb->palette=background.palette;fb->blit(1,0,0,background.images[0],false);}
		else fb->clear(1,3);
		ShotBackend shots{*fb,stg,{}};ShotBackendBinding shot_binding(shots);
		PlayerBackend player{shots,miko,action48,action32,{},{},{}};PlayerBackendBinding player_binding(player);
		std::uint64_t trace=14695981039346656037ull;
		auto tick=[&]() {
			player_unput_update_render();
			for(auto& plane:fb->pages[0]) for(auto value:plane) {trace^=value;trace*=1099511628211ull;}
		};
		auto reset=[&]() {
			fb->pages[0]=fb->pages[1];
			for(int i=0;i<SHOT_COUNT;++i) {Shots.moving[i]=false;Shots.decay_frame[i]=0;Shots.left[i]=0;Shots.top[i]=0;}
			input_lr=INPUT_NONE;input_shot=false;input_strike=false;input_bomb=false;
			player_left=304;player_invincible=false;player_is_hit=false;rem_bombs=3;rank=1;
			orb_cur_left=0;orb_cur_top=64;orb_in_portal=false;native_random_seed=1;
			shots.sound_commands.clear();player_reset();
		};
		auto moving=[]() {unsigned count=0;for(int i=0;i<SHOT_COUNT;++i) count+=Shots.moving[i];return count;};
		reset();input_lr=INPUT_RIGHT;tick();
		require(player_left==308,"Regular right movement must be four pixels");
		reset();input_shot=true;tick();
		require(moving()==0,"Standing shot must wait for its cast frame");
		tick();require(moving()==1 && Shots.left[0]==312 && Shots.top[0]==368,"Standing shot origin/timing");
		tick();tick();require(!input_shot && moving()==1,"Standing shot cooldown");
		reset();input_lr=INPUT_RIGHT;input_shot=true;
		for(int i=0;i<3;++i) tick();
		require(player_left==316 && moving()==1 && Shots.left[0]==324,"Moving shot cast/movement sequence");
		reset();input_strike=true;tick();
		require(player_deflecting && !player_sliding,"Swing start");
		orb_cur_left=player_left;orb_cur_top=353;orb_player_hittest(OR_NONE);
		require(!player_is_hit,"Swing grants orb invincibility on its initial invisible frames");
		orb_cur_top=64;
		for(int i=0;i<12;++i) tick();
		require(player_deflecting,"Normal-rank swing deflection duration");
		tick();require(!player_deflecting,"Normal-rank swing deflection cutoff");
		for(int i=0;i<11;++i) tick();
		require(!input_strike && !player_sliding,"Swing completion");
		reset();input_lr=INPUT_RIGHT;input_strike=true;tick();
		require(player_sliding && player_left==308,"Slide entry");
		input_strike=false;
		for(int i=0;i<13;++i) tick();
		require(!player_sliding && player_left==386,"Original inclusive 13-step slide duration");
		// Release, then re-press at the slide's combo window: moving flip kick.
		reset();input_lr=INPUT_RIGHT;input_strike=true;tick();
		input_strike=false;for(int i=0;i<12;++i) tick();
		input_lr=INPUT_NONE;input_strike=true;tick();
		require(player_deflecting && !player_sliding && player_left==386,"Slide-to-flipkick transition");
		input_strike=false;
		for(int i=0;i<28;++i) tick();
		require(player_left==442 && !player_deflecting,"Moving flipkick trajectory/deflection");
		input_shot=true;tick(); // continue to shot combo
		for(int i=0;i<20;++i) tick();
		require(moving()==7,"Shot combo must emit seven shots");
		require(player_left==444,"The final flipkick frame still moves before the combo transition");
		const int origins[]={444,460,452,436,452,468,452};
		for(int i=0;i<7;++i) require(Shots.left[i]==origins[i],"Shot combo origin pattern");
		unsigned shot_sounds=0;for(auto id:shots.sound_commands) shot_sounds+=id==1;
		require(shot_sounds==7,"Shot combo SE command count");
		// Exercise all three special slots: moving flipkick -> stationary
		// flipkick -> slidekick. Held input must not create a fourth special.
		reset();input_lr=INPUT_RIGHT;input_strike=true;tick();
		input_strike=false;for(int i=0;i<12;++i) tick();
		input_lr=INPUT_NONE;input_strike=true;tick();
		input_strike=false;for(int i=0;i<28;++i) tick();
		input_lr=INPUT_LEFT;input_strike=true;tick();
		require(player_left==444 && player_deflecting,"Second special stationary flipkick entry");
		input_strike=false;for(int i=0;i<28;++i) tick();
		require(player_left==444,"Stationary flipkick must not move");
		input_lr=INPUT_RIGHT;input_strike=true;tick();
		for(int i=0;i<14;++i) tick();
		unsigned strike_sounds=0;for(auto id:shots.sound_commands) strike_sounds+=id==11;
		require(player_left==528 && !player_deflecting && !player_sliding && strike_sounds==4,"Third special finishes without extending the combo");
		std::uint64_t hash=14695981039346656037ull;
		for(auto& plane:fb->pages[0]) for(auto value:plane) {hash^=value;hash*=1099511628211ull;}
		require(trace==(argc==2 ? 0x874dd2c4e9cd1c6eull : 0xe5fe831898794205ull),"Player action framebuffer trace regression");
		std::cout << "Original shot, swing, slide, seven-shot combo and three-special cap passed; framebuffer " << std::hex << hash << "; frame trace " << trace << '\n';
		reset();input_bomb=true;bool rejected=false;
		try {tick();} catch(const std::runtime_error&) {rejected=true;}
		require(rejected,"Unconnected pellet/bomb systems must not silently succeed");
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}
