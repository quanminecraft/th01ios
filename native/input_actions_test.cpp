#include "input_backend.hpp"
#include "player_backend.hpp"
#include "th01/main/player/player.hpp"
#include "th01/main/player/orb.hpp"
#include "th01/main/player/shot.hpp"
#include <memory>
#include <iostream>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
static SpriteSet bank(unsigned count,int w,int h) {
	SpriteSet set;for(unsigned i=0;i<count;++i) set.images.push_back({w,h,Bytes(w*h,2),Bytes(w*h,1)});return set;
}
int main() {
	try {
		auto fb=std::make_unique<Framebuffer>();
		auto stg=bank(34,32,32),miko=bank(28,32,32),action48=bank(28,48,48),action32=bank(6,48,32);
		ShotBackend shots{*fb,stg,{}};ShotBackendBinding shot_binding(shots);
		PlayerBackend player{shots,miko,action48,action32,{},{},{}};PlayerBackendBinding player_binding(player);
		InputKeys keys;
		auto tick=[&]() {set_input_keys(keys);input_sense(false);player_unput_update_render();++bomb_doubletap_frame;};
		auto reset=[&]() {
			keys={};set_input_keys(keys);input_reset_sense();bomb_doubletap_frame=0;
			player_left=304;player_is_hit=false;orb_cur_left=0;orb_cur_top=64;orb_in_portal=false;
			for(int i=0;i<8;++i) {Shots.moving[i]=false;Shots.decay_frame[i]=0;}
			shots.sound_commands.clear();player_reset();
		};
		auto count=[]() {unsigned n=0;for(int i=0;i<8;++i) n+=Shots.moving[i];return n;};
		reset();keys.right=true;tick();require(player_left==308,"Arrow snapshot must drive original player motion");
		keys.right=false;keys.shot=true;for(int i=0;i<60;++i) tick();
		require(count()==1,"Holding shot must produce one cast, not synthetic autofire");
		keys.shot=false;tick();keys.shot=true;for(int i=0;i<4;++i) tick();
		require(count()==2,"Shot re-press must produce the second cast");
		keys.strike=true;for(int i=0;i<60;++i) tick();require(!player_deflecting,"Held strike must finish its single swing");
		keys.strike=false;tick();keys.strike=true;tick();require(player_deflecting,"Strike re-press must start another swing");
		reset();keys.right=true;keys.strike=true;tick();
		keys.strike=false;for(int i=0;i<12;++i) tick();
		keys.right=false;keys.strike=true;tick();require(player_deflecting && player_left==386,"Physical slide-to-flipkick input window");
		keys.strike=false;for(int i=0;i<28;++i) tick();
		keys.shot=true;tick();for(int i=0;i<20;++i) tick();
		require(player_left==444 && count()==7,"Physical input sequence must drive the seven-shot combo");
		std::cout << "Original sampled input -> player motion/cast/swing/combo integration passed\n";
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}
