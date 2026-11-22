#include "pellet_backend.hpp"
#include <memory>
#include <iostream>
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main() {
	try {
		auto fb=std::make_unique<th01_native::Framebuffer>();th01_native::SpriteSet sprites;
		for(unsigned i=0;i<34;++i) sprites.images.push_back({32,32,th01_native::Bytes(1024,2),th01_native::Bytes(1024,1)});
		th01_native::ShotBackend shots{*fb,sprites,{}};th01_native::ShotBackendBinding shot_binding(shots);
		auto reset=[&]() {
			fb->clear(0,0);fb->clear(1,0);page_access(0);Pellets.native_reset_all();
			for(int i=0;i<8;++i) {Shots.left[i]=0;Shots.top[i]=0;Shots.moving[i]=false;Shots.decay_frame[i]=0;}
			rank=RANK_EASY;native_pellet_speed=0;pellet_interlace=false;pellet_destroy_score_delta=0;
			player_left=304;player_is_hit=false;player_deflecting=false;player_sliding=false;player_invincible=false;
			orb_in_portal=false;orb_cur_left=0;orb_cur_top=64;native_random_seed=1;shots.sound_commands.clear();
		};
		for(unsigned difficulty=0;difficulty<4;++difficulty) {
			reset();rank=difficulty;Pellets.add_group(100,100,PG_1,56);
			require(Pellets.native_alive_count()==1 && Pellets.native_at(0).velocity.y.v==56+int(difficulty)*6,"Rank speed increments");
		}
		reset();rank=RANK_NORMAL;native_pellet_speed=20;Pellets.add_group(100,100,PG_1,56);
		require(Pellets.native_at(0).velocity.y.v==93,"Resident speed multiplier");
		reset();Pellets.add_group(100,100,PG_1,-32);require(Pellets.native_at(0).velocity.y.v==16,"Minimum speed clamp");
		reset();for(int i=0;i<101;++i) Pellets.add_group(100,100,PG_1,56);require(Pellets.native_alive_count()==100,"100-slot capacity");
		reset();Pellets.add_group(640,100,PG_1,56);Pellets.add_group(-9,100,PG_1,56);Pellets.add_group(100,55,PG_1,56);Pellets.add_group(100,401,PG_1,56);
		require(Pellets.native_alive_count()==0,"Group spawn bounds");
		Pellets.add_group(-8,56,PG_1,56);require(Pellets.native_alive_count()==1,"Inclusive spawn margins");
		reset();Pellets.add_group(100,100,PG_3_SPREAD_WIDE,56);
		require(Pellets.native_alive_count()==3 && Pellets.native_at(0).velocity.x.v==0 && Pellets.native_at(0).velocity.y.v==56,"Three-way center");
		require(Pellets.native_at(1).velocity.x.v==-13 && Pellets.native_at(1).velocity.y.v==54 && Pellets.native_at(2).velocity.x.v==12,"Spread retains negative-rounding asymmetry");
		reset();player_left=376;Pellets.add_group(100,100,PG_1_AIMED,56);
		require(Pellets.native_at(0).velocity.x.v==39 && Pellets.native_at(0).velocity.y.v==39,"Original aimed target point");
		reset();Pellets.add_group(100,100,PG_1_RANDOM_WIDE,56);
		require(native_random_seed==0x015a4e36 && Pellets.native_at(0).velocity.x.v==8 && Pellets.native_at(0).velocity.y.v==55,"Random spread RNG/angle");
		reset();Pellets.add_group(100,100,PG_1,56);Pellets.unput_update_render();
		require(Pellets.native_at(0).cur_top.v==1656 && Pellets.native_alive_count()==1,"Q12.4 linear motion");
		reset();Pellets.spawn_with_cloud=true;Pellets.add_group(100,100,PG_1,56);Pellets.reset_nonclouds();
		require(Pellets.native_at(0).cloud_frame==1 && !Pellets.native_at(0).moving && Pellets.native_alive_count()==1,"Noncloud reset preserves active cloud");
		for(int i=0;i<7;++i) Pellets.unput_update_render();require(!Pellets.native_at(0).moving,"Cloud delay before final keyframe");
		Pellets.unput_update_render();require(Pellets.native_at(0).moving && Pellets.native_at(0).cloud_frame==0 && Pellets.native_at(0).cur_top.v==1656,"Cloud starts motion on eighth update");
		reset();Pellets.add_single(100,100,0,56,PM_GRAVITY,16);Pellets.unput_update_render();
		require(Pellets.native_at(0).velocity.y.v==16 && Pellets.native_at(0).cur_top.v==1616,"Gravity motion");
		reset();Pellets.add_single(100,66,128,56,PM_FALL_STRAIGHT_FROM_TOP_THEN_REGULAR,32);Pellets.unput_update_render();
		require(Pellets.native_at(0).motion_type==PM_REGULAR && Pellets.native_at(0).velocity.x.v==0 && Pellets.native_at(0).cur_top.v==1088,"Fall-straight top transition");
		reset();Pellets.add_single(100,69,0,56,PM_BOUNCE_FROM_TOP_THEN_GRAVITY,16);Pellets.unput_update_render();
		require(Pellets.native_at(0).motion_type==PM_GRAVITY && Pellets.native_at(0).velocity.x.v==-56 && Pellets.native_at(0).cur_top.v==1104,"Top bounce transition");
		reset();Pellets.add_single(100,100,0,0,PM_SPIN,16,100,100);Pellets.unput_update_render();
		require(Pellets.native_at(0).cur_left.v==1856 && Pellets.native_at(0).cur_top.v==1600 && Pellets.native_at(0).spin_center.x.v==1616 && Pellets.native_at(0).angle==4,"Spin center, radius and angle");
		reset();Pellets.add_single(100,100,0,56,PM_CHASE,56);Pellets.unput_update_render();
		require(Pellets.native_at(0).velocity.x.v==55 && Pellets.native_at(0).velocity.y.v==1,"Chase adjusts by one subpixel");
		reset();Pellets.add_single(100,100,0,56,PM_SLING_AIMED,56);
		for(int i=0;i<32;++i) Pellets.unput_update_render();require(Pellets.native_at(0).from_group==PG_NONE,"Sling includes angle 256");
		Pellets.unput_update_render();require(Pellets.native_at(0).from_group==PG_1_AIMED && Pellets.native_at(0).angle==0,"Sling switches to straight aim on update 33");
		reset();Pellets.add_single(100,100,0,160);Pellets.unput_update_render();require(Pellets.native_at(0).cur_left.v==1728,"Positive velocity clamp");
		reset();Pellets.add_single(100,100,128,160);Pellets.unput_update_render();require(Pellets.native_at(0).cur_left.v==1440,"Negative velocity remains unclamped");
		reset();Pellets.add_group(308,368,PG_1,16);Pellets.unput_update_render();require(player_is_hit && Pellets.native_alive_count()==0,"Pellet/player hit");
		reset();player_invincible=true;Pellets.add_group(308,368,PG_1,16);Pellets.unput_update_render();require(!player_is_hit && Pellets.native_alive_count()==1,"Player invincibility");
		reset();player_sliding=true;Pellets.add_group(308,368,PG_1,16);Pellets.unput_update_render();require(!player_is_hit,"Slide hitbox top");
		reset();player_deflecting=true;Pellets.add_single(312,353,0,56,PM_GRAVITY,16);Pellets.unput_update_render();
		require(Pellets.native_at(0).velocity.x.v==-128 && Pellets.native_at(0).velocity.y.v==0 && Pellets.native_at(0).motion_type==PM_REGULAR && Pellets.native_at(0).not_rendered,"Player deflection and invisible frame");
		reset();orb_cur_left=100;orb_cur_top=100;Pellets.add_group(100,100,PG_1,56);Pellets.unput_update_render();
		require(Pellets.native_at(0).velocity.y.v==28 && Pellets.native_at(0).decay_frame==2 && pellet_destroy_score_delta==10,"Orb collision halves velocity and starts decay");
		for(int i=0;i<19;++i) Pellets.unput_update_render();require(Pellets.native_at(0).moving && Pellets.native_at(0).decay_frame==21,"Decay before final removal");
		Pellets.unput_update_render();require(!Pellets.native_at(0).moving && Pellets.native_alive_count()==0,"Decay removal");
		reset();Pellets.add_group(100,100,PG_1,56);Pellets.decay();require(Pellets.native_at(0).velocity.y.v==37 && pellet_destroy_score_delta==10,"Bomb decay truncation");
		Pellets.decay();require(pellet_destroy_score_delta==10,"Already-decaying pellet is not scored twice");
		reset();Pellets.add_group(100,100,PG_1,56);Shots.add(100,108);Pellets.unput_update_render();
		require(Pellets.native_alive_count()==0 && !Shots.moving[0] && Shots.decay_frame[0]==2,"Shot collision and same-frame shot decay update");
		reset();pellet_interlace=true;Pellets.add_group(100,100,PG_1,56);Pellets.add_group(200,100,PG_1,56);Pellets.unput_update_render();
		require(Pellets.native_at(0).cur_top.v==1656 && Pellets.native_at(1).cur_top.v==1656 && Pellets.native_at(0).prev_left.v==-1 && Pellets.native_at(1).prev_left.v==3200,"Interlace moves all pellets but draws one field");
		std::cout << "Original pellet spawn, seven motion modes, cloud, collision, decay and interlace tests passed\n";
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}
