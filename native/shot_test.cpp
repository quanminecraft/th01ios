#include "th01/main/player/orb.hpp"
#include "th01/main/player/shot.hpp"
#include "shot_backend.hpp"
#include <memory>
#include <iostream>
using namespace th01_native;
static void require(bool value) {if(!value) throw std::runtime_error("Shot test failed");}
static void reset() {
	for(int i=0;i<SHOT_COUNT;++i) {Shots.left[i]=0;Shots.top[i]=0;Shots.moving[i]=false;Shots.decay_frame[i]=0;}
	orb_in_portal=true;
}
int main() {
	auto fb=std::make_unique<Framebuffer>();
	SpriteSet sprites;
	sprites.images.resize(2);
	sprites.images[1]={32,32,Bytes(1024,2),Bytes(1024,1)};
	sprites.images[1].opaque[0]=0;
	ShotBackend backend{*fb,sprites,{}};
	ShotBackendBinding binding(backend);
	reset();fb->clear(0,7);fb->clear(1,3);
	Shots.add(15,100);Shots.unput_update_render();
	require(Shots.top[0]==88 && Shots.moving[0] && backend.sound_commands==std::vector<unsigned>{1});
	require(fb->pixel(0,8,88)==7 && fb->pixel(0,9,88)==2 && fb->pixel(0,0,100)==3 && fb->pixel(0,31,100)==3);
	Shots.unput_and_reset();require(!Shots.moving[0] && fb->pixel(0,9,88)==3);
	reset();backend.sound_commands.clear();
	Shots.add(-1,100);Shots.add(640,100);require(backend.sound_commands.empty());
	for(int i=0;i<9;++i) Shots.add(0,100);
	require(backend.sound_commands.size()==8);
	reset();Shots.add(100,70);Shots.unput_update_render();require(!Shots.moving[0]);
	reset();orb_in_portal=false;orb_cur_left=100;orb_cur_top=100;orb_force_new(-8,OF_IMMEDIATE);orb_velocity_y_update();
	Shots.add(109,100);require(Shots.hittest_orb(0,100,100));
	require(!Shots.moving[0] && orb_velocity_x==OVX_4_LEFT && orb_cur_left==96 && orb_cur_top==86 && orb_force==-14);
	reset();Shots.add(84,100);require(!Shots.hittest_orb(0,100,100)); // strict orb edge
	reset();Shots.add(100,100);backend.sound_commands.clear();
	require(Shots.hittest_pellet(114,100)); // inclusive pellet edge
	require(!Shots.moving[0] && Shots.decay_frame[0]==1 && backend.sound_commands==std::vector<unsigned>{16});
	for(int i=0;i<7;++i) Shots.unput_update_render();
	require(!Shots.moving[0] && Shots.decay_frame[0]==0);
	reset();Shots.add(120,130);require(Shots.hittest_boss(100,100,20,30));
	require(Shots.decay_frame[0]==1);
	// Horizontal EGC wrap at the last partial shot cell restores next row.
	fb->clear(0,7);fb->clear(1,3);ptn_sloppy_unput_quarter_16(639,90);
	require(fb->pixel(0,639,90)==3 && fb->pixel(0,0,91)==3 && fb->pixel(0,16,91)==7);
	fb->clear(0,7);fb->clear(1,3);page_access(1);ptn_put_quarter_8(8,8,1,0);
	require(current_render_page()==1 && fb->pixel(1,9,8)==2 && fb->pixel(0,9,8)==7);
	fb->clear(1,3);ptn_unput_quarter_8(8,8,1,0);
	require(current_render_page()==0 && fb->pixel(0,9,8)==3);
	std::cout << "Original shot update, collision, capacity, decay and native rendering tests passed\n";
}
