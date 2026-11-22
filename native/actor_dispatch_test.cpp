#include "pellet_backend.hpp"
#include "obstacle_backend.hpp"
#include "stage_pellet_binding.hpp"
#include "stage_tick.hpp"
#include "regular_frame.hpp"
#include "item_runtime.hpp"
#include "extend_backend.hpp"
#include "timer_backend.hpp"
#include "invincibility_backend.hpp"
#include "laser_backend.hpp"
#include "pause_backend.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main() {
	try {
		auto fb=std::make_unique<Framebuffer>();SpriteSet sprites;
		for(unsigned i=0;i<34;++i) sprites.images.push_back({32,32,Bytes(1024,2),Bytes(1024,1)});
		ShotBackend shots{*fb,sprites,{}};ShotBackendBinding shot_binding(shots);
		PlayerBackend player{shots,sprites,sprites,sprites,{},{},{}};PlayerBackendBinding player_binding(player);
		PelletSpeedBinding speed_binding(player);
		Bytes font_data(0x46800);FontRom font(std::move(font_data));
		CardRenderer renderer{*fb,sprites,font,{}};
		CardEvents events;events.renderer=&renderer;CardBackendBinding card_binding(events);
		StagePelletBinding pellet_binding(events);
		auto dispatch=events.pellet_dispatch;
		unsigned calls=0;std::vector<std::uint32_t> seeds;
		events.pellet_dispatch=[&](screen_x_t left,screen_y_t top,pellet_group_t group,subpixel_t speed) {
			require(events.pellets.size()==calls+1,"Observer records each call before dispatch");
			dispatch(left,top,group,speed);++calls;seeds.push_back(native_random_seed);
			require(Pellets.native_alive_count()==int(calls),"Bullet exists before original caller resumes");
		};
		auto reset=[&]() {
			Pellets.native_reset_all();native_pellet_speed=0;native_random_seed=1;calls=0;seeds.clear();events.pellets.clear();
			rank=RANK_NORMAL;player_left=304;orb_cur_left=600;orb_cur_top=368;orb_in_portal=false;
			fb->clear(0,3);fb->clear(1,3);page_access(0);
		};
		reset();StageLayout layout;
		layout.obstacles={{100,100,OT_TURRET_QUICK_1_RANDOM_NARROW_AIMED,0},{200,100,OT_TURRET_QUICK_1_RANDOM_NARROW_AIMED,0}};
		load_obstacles(layout);obstacles_update_and_render(true);
		for(int i=0;i<109;++i) obstacles_update_and_render(false);
		require(calls==0 && Pellets.native_alive_count()==0,"No active bullets during turret warmup");
		obstacles_update_and_render(false);
		require(calls==2 && seeds[0]==0x015a4e36u && seeds[1]==seeds[0]*0x015a4e35u+1,"Two turrets consume RNG synchronously in source order");
		require(Pellets.native_at(0).cur_left.v==112*16 && Pellets.native_at(1).cur_left.v==212*16,"Turret dispatch preserves slot/origin order");
		reset();rank=RANK_LUNATIC;stage_num=10;score=0;cardcombo_cur=0;cardcombo_max=0;card_flip_cycle=1;
		stage_cleared=false;player_is_hit=false;test_damage=false;bomb_damaging=false;
		StageLayout cards_layout;cards_layout.cards={{64,96,0,0}};load_cards(cards_layout);capture_card_backgrounds(renderer);
		orb_cur_left=64;orb_cur_top=96;cards_hittest(0);
		for(int i=0;i<24;++i) cards_update_and_render();
		require(calls==0 && Pellets.native_alive_count()==0,"Card revenge waits until frame 25");
		auto seed_before=native_random_seed;cards_update_and_render();
		require(calls==1 && seeds[0]==seed_before*0x015a4e35u+1,"Random revenge creates bullet and consumes RNG inside card update");
		require(Pellets.native_at(0).cur_left.v==76*16 && Pellets.native_at(0).cur_top.v==108*16,"Card revenge creates actual bullet at original origin");
		reset();stage_num=1;player_is_hit=false;player_invincible=false;stage_cleared=false;
		orb_cur_left=600;orb_prev_left=600;orb_cur_top=300;orb_prev_top=300;
		orb_velocity_x=OVX_4_RIGHT;orb_rotation_frame=0;orb_force_new(0,OF_IMMEDIATE);
		for(int i=0;i<8;++i) {Shots.moving[i]=false;Shots.decay_frame[i]=0;Shots.left[i]=0;Shots.top[i]=0;}
		StageLayout frame_layout;frame_layout.cards={{64,96,0,0}};frame_layout.obstacles=layout.obstacles;
		load_cards(frame_layout);capture_card_backgrounds(renderer);load_obstacles(frame_layout);obstacles_update_and_render(true);
		for(int i=0;i<109;++i) obstacles_update_and_render(false);
		unsigned waits=0;
		StageTimingBinding timing([&](unsigned frames) {
			++waits;require(frames==1 && orb_cur_left==600+int(waits)*4,"Orb moves before original frame wait");
			if(waits==1) require(Pellets.native_alive_count()==0 && native_random_seed==1,"Wait precedes turret spawn and its RNG");
			if(waits==2) require(Pellets.native_at(0).cur_top.v==112*16,"Wait precedes existing bullet motion");
		});
		orb_and_pellets_and_stage_unput_update_render__vsync_wait(0);
		require(calls==2 && Pellets.native_at(0).cur_top.v==112*16 && orb_prev_left==604,"Frame spawns turret bullets after pellet movement and records orb position");
		orb_and_pellets_and_stage_unput_update_render__vsync_wait(0);
		require(waits==2 && calls==2 && Pellets.native_at(0).cur_top.v>112*16 && orb_prev_left==608,"Next shared frame advances active bullets exactly once");
		reset();reset_item_storage();native_point_value=0;score=0;stage_cleared=false;player_is_hit=false;
		player_left=304;player_invincibility_time=2;player_invincible=false;player_reset();
		orb_cur_left=600;orb_prev_left=600;orb_cur_top=300;orb_prev_top=300;orb_force_new(0,OF_IMMEDIATE);
		for(int i=0;i<8;++i) {Shots.moving[i]=false;Shots.decay_frame[i]=0;Shots.left[i]=0;Shots.top[i]=0;}
		StageLayout plain;plain.cards={{64,96,0,0}};load_cards(plain);capture_card_backgrounds(renderer);load_obstacles(plain);
		set_input_keys({});input_reset_sense();bomb_frame=10;bomb_doubletap_frame=20;mode_test=false;
		items_point[0]={330,350,0,2,IF_FALL,{0}};
		RegularFrameState frame;frame.frame_rand=3;
		std::vector<int> order;RegularFrameHooks hooks;
		hooks.timer=[&]() {order.push_back(1);require(frame.frame_since_start_of_binary==1 && bomb_frame==11,"Counters advance before timer");};
		hooks.invincibility=[&](bool16 active) {order.push_back(2);require(active && player_invincibility_time==1,"Invincibility decrements before effects");};
		hooks.shootout_lasers=[&]() {order.push_back(3);};
		hooks.score_extend=[&]() {order.push_back(5);};hooks.pause_menu=[]() {return false;};
		StageTimingBinding frame_timing([&](unsigned n) {require(n==1,"Frame wait length");order.push_back(4);});
		InputKeys right;right.right=true;
		require(regular_frame(frame,0,right,hooks)==FrameResult::running,"Composed regular frame continues");
		require(player_left==308 && native_point_value==1000 && score==1000 && items_point[0].flag_state.collect_time==15,"Player moves before item pickup and popup update");
		require(order==std::vector<int>({1,2,3,4,5}) && frame.frame_rand==4 && bomb_doubletap_frame==21,"Original frame integration order and counters");
		order.clear();hooks.invincibility=[&](bool16 active) {order.push_back(2);require(!active && player_invincibility_time==0,"Invincibility expires on next frame");};
		require(regular_frame(frame,0,{},hooks)==FrameResult::running && order==std::vector<int>({2,3,4,5}),"Timer runs once every four frames");
		order.clear();pellet_destroy_score_delta=123;auto before_award=score;
		hooks.score_extend=[&]() {order.push_back(5);require(score==before_award && pellet_destroy_score_delta==123,"Life extend check precedes pellet destroy award");};
		require(regular_frame(frame,0,{},hooks)==FrameResult::running && score==before_award+123 && pellet_destroy_score_delta==0,"Frame credits and clears pellet destroy points after extend");
		order.clear();hooks.pause_menu=[&]() {order.push_back(6);return true;};InputKeys escape;escape.escape=true;pellet_destroy_score_delta=17;
		require(regular_frame(frame,0,escape,hooks)==FrameResult::quit && order==std::vector<int>({2,3,4,6}) && pellet_destroy_score_delta==17,"Pause quit exits before extend and score award");
		player_is_hit=true;auto stopped=frame.frame_rand;
		require(regular_frame(frame,0,{},hooks)==FrameResult::hit && frame.frame_rand==stopped,"Hit exits life loop before another frame");
		player_is_hit=false;set_input_keys({});input_reset_sense();pellet_destroy_score_delta=0;
		order.clear();paused=false;RegularFrameHooks boss_hooks;
		boss_hooks.timer=[&]() {order.push_back(1);};boss_hooks.invincibility=[&](bool16) {order.push_back(2);};
		boss_hooks.boss_update=[&]() {order.push_back(7);};boss_hooks.shootout_lasers=[&]() {order.push_back(3);};
		boss_hooks.score_extend=[&]() {order.push_back(5);};boss_hooks.pause_menu=[]() {return false;};frame.frame_rand=7;
		require(regular_frame(frame,4,{},boss_hooks)==FrameResult::running && order==std::vector<int>({1,2,7,3,4,5}),"Boss AI runs after invincibility and before lasers and orb/pellet wait");
		frame.lives=4;frame.frame_rand=8;extend_next=1;score=400000;native_pellet_speed=0;std::int8_t saved_lives=4;unsigned life_hud=0;
		ExtendBackend life_backend{frame.lives,saved_lives,[&](int previous) {require(previous==4,"Extend HUD previous lives");++life_hud;}};ExtendBackendBinding life_binding(life_backend);
		RegularFrameHooks live_hooks;live_hooks.timer=[]() {};live_hooks.shootout_lasers=[]() {};live_hooks.invincibility=[](bool16) {};live_hooks.pause_menu=[]() {return false;};
		live_hooks.score_extend=score_extend_update_and_render;
		require(regular_frame(frame,0,{},live_hooks)==FrameResult::running && frame.lives==5 && saved_lives==5 && life_hud==1 && native_pellet_speed==1,"Composed frame performs original life extend and speed increase");
		frame.frame_rand=1800-frame.lives*200-rem_bombs*50-1;
		require(regular_frame(frame,0,{},live_hooks)==FrameResult::running && native_pellet_speed==2,"Original frame cadence raises actual resident pellet speed");
		TimerBackend timer{*fb,{}, {},[]() {}};TimerBackendBinding timer_binding(timer);
		timer_init_for(0,ROUTE_MAKAI);page_access(0);timer_bg_snap_and_put();live_hooks.timer=timer_tick_and_put;
		frame.frame_rand=3;regular_frame(frame,0,{},live_hooks);
		require(stage_timer==998,"Composed gameplay frame invokes actual stage timer on fourth frame");
		player_invincibility_time=2;native_random_seed=1;live_hooks.invincibility=invincibility_sprites_update_and_render;
		regular_frame(frame,0,{},live_hooks);
		require(native_random_seed==0xb6a1d149u && player_invincible && player_invincibility_time==1,"Composed frame invokes original invincibility visuals/RNG");
		live_hooks.shootout_lasers=update_shootout_lasers;shootout_lasers[0].spawn(100,100,200,100,64,2,100,1);
		regular_frame(frame,0,{},live_hooks);
		require(shootout_lasers[0].age==1 && shootout_lasers[0].ray_length==8,"Composed frame updates actual shootout lasers once");
		BombBackend pause_palette{*fb,sprites,sprites,[]() {}};BombBackendBinding pause_palette_binding(pause_palette,player);
		std::uint16_t continues=0;unsigned menu_wait=0;PauseBackend pause{{},[&](unsigned n) {menu_wait+=n;},continues};
		pause.poll=[]() {InputKeys enter;enter.enter=true;return enter;};PauseBackendBinding pause_binding(pause);
		live_hooks.pause_menu=pause_menu;auto frames_before=frame.frame_since_start_of_binary;auto seed_before_pause=native_random_seed;InputKeys pause_key;pause_key.escape=true;
		require(regular_frame(frame,0,pause_key,live_hooks)==FrameResult::running && !paused && menu_wait==20 && continues==0,"Composed frame invokes actual pause menu and resumes");
		require(frame.frame_since_start_of_binary==frames_before+1 && native_random_seed==seed_before_pause,"Blocking pause does not advance gameplay frames or consume RNG");
		std::cout << "Synchronous original turret/card dispatch, active bullets and RNG order passed\n";
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}
