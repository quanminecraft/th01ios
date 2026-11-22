#include "th01/main/player/orb.hpp"
#include <iostream>
#include <cmath>
#include <cstring>
extern double orb_velocity_y;
static void require(bool condition) { if(!condition) throw std::runtime_error("Orb physics test failed"); }
int main() {
	static_assert(sizeof(screen_x_t)==2 && sizeof(bool16)==2 && sizeof(orb_velocity_x_t)==2 && sizeof(orb_force_frame)==2);
	native_random_seed=1;
	require(irand()==346 && native_random_seed==0x015a4e36);
	orb_force_new(-8,OF_IMMEDIATE);
	orb_force_frame=4;require(orb_velocity_y_update()==-8 && orb_velocity_y_get()==-8);
	orb_force_frame=5;require(orb_velocity_y_update()==-7);
	orb_force_frame=150;
	// Upstream clamps the stored velocity, but returns the UNCLAMPED
	// gravity result to movement. Retain that observable quirk.
	require(orb_velocity_y_update()==22 && orb_velocity_y_get()==16);
	orb_force_new(0,OF_SHOT);require(orb_force==-2 && orb_force_frame==0);
	orb_velocity_y=-10;orb_force_frame=19;orb_force_new(0,OF_BOUNCE_FROM_TOP);
	require(orb_force==6 && orb_force_frame==0);
	orb_cur_left=2;orb_velocity_x=OVX_4_LEFT;orb_move_x(orb_velocity_x);
	require(orb_cur_left==-2 && orb_velocity_x==OVX_4_RIGHT);
	orb_cur_left=606;orb_velocity_x=OVX_4_RIGHT;orb_move_x(orb_velocity_x);
	require(orb_cur_left==610 && orb_velocity_x==OVX_4_LEFT);
	orb_velocity_y=-5;orb_force_frame=5;orb_velocity_x=OVX_4_LEFT;
	orb_force_new(0,OF_BOUNCE_FROM_SURFACE);
	require(std::abs(orb_force-3.9)<1e-12 && orb_force_frame==0);
	// Stable cross-architecture trace of isolated free motion and edge bounce.
	// This deliberately excludes player/card/obstacle collision and is not a
	// claim of original-game replay equivalence.
	orb_cur_left=ORB_LEFT_START;orb_cur_top=ORB_TOP_START;
	orb_velocity_x=ORB_VELOCITY_X_START;orb_force=ORB_FORCE_START;orb_force_frame=0;
	std::uint64_t hash=14695981039346656037ull;
	auto feed=[&](std::uint64_t value) {for(unsigned i=0;i<8;++i) {hash^=(value>>(i*8))&255;hash*=1099511628211ull;}};
	for(unsigned frame=0;frame<10000;++frame) {
		++orb_force_frame;
		orb_move_x(orb_velocity_x);
		orb_cur_top+=orb_velocity_y_update();
		if(orb_cur_top>ORB_TOP_MAX) {orb_force_new(0.78,OF_BOUNCE_FROM_SURFACE);orb_cur_top=ORB_TOP_MAX;}
		if(orb_cur_top<ORB_TOP_MIN) {orb_force_new(0,OF_BOUNCE_FROM_TOP);orb_cur_top=ORB_TOP_MIN;}
		feed(std::uint16_t(orb_cur_left));feed(std::uint16_t(orb_cur_top));feed(orb_velocity_x);feed(orb_force_frame);
		std::uint64_t force;std::memcpy(&force,&orb_force,8);feed(force);
	}
	require(hash==0xc1b2b0da96abb803ull);
	std::cout << "Original orb physics tests passed; 10000-frame trace " << std::hex << hash << '\n';
}
