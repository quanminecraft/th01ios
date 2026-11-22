#include "laser_backend.hpp"
#include <memory>
#include <iostream>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main() {
	try {
		static_assert(sizeof(LaserPixel)==4);
		auto fb=std::make_unique<Framebuffer>();SpriteSet sprites;ShotBackend shots{*fb,sprites,{}};ShotBackendBinding binding(shots);
		auto reset=[&]() {fb->clear(0,3);fb->clear(1,5);page_access(0);player_left=304;player_is_hit=false;player_invincible=false;};
		reset();CShootoutLaser laser{};laser.spawn(100,100,200,100,64,2,3,1);
		require(laser.alive && laser.damaging && laser.ray_length==0 && laser.step_x.v==256 && laser.step_y.v==0 && laser.ray_extend_speed==8 && laser.age==0,"Original horizontal spawn state");
		laser.spawn(200,200,300,300,8,7,99,8);require(laser.origin_left.v==25600 && laser.width_cel==0,"Spawning alive slot does nothing");
		laser.update_hittest_and_render();
		require(laser.ray_length==8 && laser.age==1 && fb->pixel(0,100,100)==2 && fb->pixel(0,103,100)==2 && fb->pixel(0,104,100)==3,"Original cached draw omits final segment flush");
		laser.update_hittest_and_render();laser.update_hittest_and_render();
		require(laser.ray_length==24 && laser.ray_start_left.v==25600,"Ray extends until moveout age");
		laser.update_hittest_and_render();
		require(laser.ray_length==24 && laser.ray_start_left.v==108*256 && laser.age==4 && fb->pixel(0,100,100)==5,"Moveout advances start and restores old segment from back page");
		laser.unput_and_reset();require(!laser.alive && fb->pixel(0,108,100)==5,"Native full-ray reset restores actual coordinates safely");
		reset();CShootoutLaser downward{};downward.spawn(100,390,100,400,64,2,0,1);
		downward.update_hittest_and_render();downward.update_hittest_and_render();require(!downward.alive,"Moving ray exits bottom boundary");
		reset();CShootoutLaser hit{};hit.spawn(308,380,308,399,8,2,100,1);hit.update_hittest_and_render();
		require(!player_is_hit,"Single cached ray pixel has no flush or collision yet");
		hit.update_hittest_and_render();require(player_is_hit,"Flushed ray segment hits original player hitbox");
		reset();player_invincible=true;CShootoutLaser safe{};safe.spawn(308,380,308,399,128,2,100,8);safe.update_hittest_and_render();require(!player_is_hit,"Invincibility prevents laser damage");
		player_invincible=false;safe.damaging=false;safe.update_hittest_and_render();require(!player_is_hit,"Non-damaging laser still renders without hitting");
		reset();CShootoutLaser edge{};edge.spawn(304,376,400,376,128,2,100,1);edge.update_hittest_and_render();require(!player_is_hit,"Laser hitbox excludes exact top boundary");
		require(native_laser_mask(0)==0x80 && native_laser_mask(1)==0x20,"Original preshift bitmap values, including anomalous second row");
		CShootoutLaser diagonal{};diagonal.spawn(100,100,200,200,65,2,20,4);
		require(diagonal.step_x.v==181 && diagonal.step_y.v==181 && diagonal.ray_extend_speed==8,"Integer aim and speed division truncate as original");
		reset();std::uint64_t trace=14695981039346656037ull;
		for(int frame=0;frame<70;++frame) {diagonal.update_hittest_and_render();for(auto& page:fb->pages) for(auto& plane:page) for(auto value:plane) {trace^=value;trace*=1099511628211ull;}}
		require(trace==0x3754135f4e60b242ull,"Laser framebuffer trace regression");
		std::cout << "Original laser spawn, motion, cache, hitbox and safe reset passed; trace " << std::hex << trace << '\n';
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}
