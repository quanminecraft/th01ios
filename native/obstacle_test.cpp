#include "obstacle_backend.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <memory>
#include <iostream>
#include <cmath>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
static StageObject object(int type,int left=100,int top=100) {return {left,top,static_cast<std::uint8_t>(type),0};}
int main(int argc,char** argv) {
	try {
		if(argc>3) throw std::runtime_error("Usage: obstacle_test [ASSET_DIR [SCREENSHOT.bmp]]");
		static_assert(sizeof(ObstacleState::Frame)==2 && sizeof(obstacle_type_t)==2);
		auto fb=std::make_unique<Framebuffer>();SpriteSet sprites;
		for(unsigned i=0;i<34;++i) sprites.images.push_back({32,32,Bytes(1024,std::uint8_t(i%14+1)),Bytes(1024,1)});
		ShotBackend shots{*fb,sprites,{}};ShotBackendBinding shot_binding(shots);
		PlayerBackend player{shots,sprites,sprites,sprites,{},{},{}};PlayerBackendBinding player_binding(player);
		CardEvents events;CardBackendBinding card_binding(events);
		auto reset=[&](std::initializer_list<StageObject> list) {
			fb->clear(0,3);fb->clear(1,3);page_access(0);events={};
			orb_in_portal=false;orb_cur_left=600;orb_cur_top=368;orb_prev_left=600;orb_prev_top=368;
			orb_velocity_x=OVX_4_RIGHT;orb_force_new(4,OF_IMMEDIATE);orb_velocity_y_update();native_random_seed=5;rank=RANK_NORMAL;
			StageLayout layout;layout.obstacles=list;load_obstacles(layout);obstacles_update_and_render(true);
		};
		reset({object(OT_BUMPER)});orb_cur_left=90;orb_cur_top=90;obstacles_update_and_render(false);
		require(orb_cur_top==76 && orb_velocity_x==OVX_4_LEFT && std::abs(orb_force+3.12)<1e-12 && obstacles.frame[0].since_collision==2,"Bumper teleport, reflection and first-frame cooldown");
		orb_cur_top=90;obstacles_update_and_render(false);require(orb_cur_top==90 && obstacles.frame[0].since_collision==3,"Bumper collision is blocked during cooldown");
		for(int i=0;i<4;++i) obstacles_update_and_render(false);require(obstacles.frame[0].since_collision==7,"Bumper cooldown before expiry");
		obstacles_update_and_render(false);require(obstacles.frame[0].since_collision==0,"Bumper cooldown expiry");
		reset({object(OT_BUMPER)});orb_cur_left=124;orb_cur_top=100;obstacles_update_and_render(false);require(orb_cur_top==100,"Strict bumper edge");
		orb_cur_left=90;orb_cur_top=110;orb_velocity_x=OVX_8_RIGHT;obstacles_update_and_render(false);
		require(orb_cur_top==124 && orb_velocity_x==OVX_8_RIGHT,"Bumper preserves original unhandled fast-X velocity");
		reset({object(OT_BAR_LEFT),object(OT_BAR_LEFT,100,132)});orb_cur_left=80;orb_cur_top=116;obstacles_update_and_render(false);
		require(orb_velocity_x==OVX_4_LEFT && obstacles.frame[0].since_collision==2 && obstacles.frame[1].since_collision==0,"Adjacent vertical bars must not double-negate X velocity");
		reset({object(OT_BAR_LEFT)});orb_cur_left=80;orb_cur_top=100;orb_velocity_x=OVX_0;obstacles_update_and_render(false);
		require(orb_velocity_x==OVX_0 && orb_force==-4,"Vertical bar with zero X reflects Y");
		reset({object(OT_BAR_TOP)});orb_cur_left=100;orb_cur_top=68;obstacles_update_and_render(false);require(orb_force==-4,"Top bar includes its upper outer boundary");
		reset({object(OT_BAR_TOP)});orb_cur_left=100;orb_cur_top=100;obstacles_update_and_render(false);require(orb_force==4,"Top bar excludes its tile top");
		reset({object(OT_BAR_BOTTOM)});orb_cur_left=100;orb_cur_top=132;obstacles_update_and_render(false);require(orb_force==-4,"Bottom bar includes its lower outer boundary");
		reset({object(OT_PORTAL),object(OT_PORTAL,300,200)});orb_cur_left=100;orb_cur_top=100;
		obstacles_update_and_render(false);require(orb_in_portal && obstacles.frame[0].since_collision==1,"Portal entry");
		for(int i=1;i<20;++i) obstacles_update_and_render(false);
		require(orb_in_portal && obstacles.frame[0].since_collision==20 && native_random_seed==2099511571u,"Portal destination selection and RNG call order at frame 20");
		for(int i=20;i<40;++i) obstacles_update_and_render(false);
		require(!orb_in_portal && orb_cur_left==300 && orb_cur_top==200 && orb_velocity_x==OVX_4_RIGHT && orb_force==-6 && native_random_seed==1773386673u,"Portal exit at frame 40");
		for(int i=0;i<19;++i) obstacles_update_and_render(false);require(!orb_in_portal,"Destination portal remains blocked after exit");
		obstacles_update_and_render(false);require(orb_in_portal && obstacles.frame[1].since_collision==1,"Destination can re-enter on source frame 60");
		reset({object(OT_PORTAL)});orb_cur_left=100;orb_cur_top=100;
		for(int i=0;i<20;++i) obstacles_update_and_render(false);
		std::uint32_t expected_seed=5;for(int i=0;i<17;++i) expected_seed=expected_seed*0x015a4e35u+1;
		require(native_random_seed==expected_seed,"Single-portal fallback performs original 17 candidate rolls");
		const pellet_group_t groups[]={PG_1_AIMED,PG_1_RANDOM_NARROW_AIMED,PG_2_SPREAD_WIDE_AIMED,PG_3_SPREAD_WIDE_AIMED,PG_4_SPREAD_WIDE_AIMED,PG_5_SPREAD_WIDE_AIMED};
		for(int quick=0;quick<2;++quick) for(int group=0;group<6;++group) {
			reset({object((quick ? OT_TURRET_QUICK_1_AIMED : OT_TURRET_SLOW_1_AIMED)+group)});
			int interval=quick ? 100 : 200;
			for(int frame=1;frame<interval+10;++frame) {
				obstacles_update_and_render(false);require(events.pellets.empty(),"Turret must wait through warmup");
				if(frame==interval) require(fb->pixel(0,100,100)==11 && fb->pixel(1,100,100)==11,"Turret warmup draws both pages");
			}
			obstacles_update_and_render(false);
			require(events.pellets.size()==1 && events.pellets[0].group==groups[group] && events.pellets[0].left==112 && events.pellets[0].top==112 && events.pellets[0].speed==56,"Turret group/origin/speed at firing keyframe");
			for(int i=0;i<7;++i) obstacles_update_and_render(false);
			require(obstacles.frame[0].fire_cycle==0 && fb->pixel(0,100,100)==10 && fb->pixel(1,100,100)==10,"Turret cooldown and background-page reset");
		}
		reset({object(OT_TURRET_QUICK_1_AIMED)});rank=RANK_EASY;
		for(int i=0;i<500;++i) obstacles_update_and_render(false);require(events.pellets.empty() && obstacles.frame[0].fire_cycle==0,"Easy rank disables turret timer");
		reset({});
		if(argc>=2) {
			auto archive=load_archive(argv[1]);auto real=read_sprites(asset(archive,"STG.PTN"));
			auto layout=read_scene(asset(archive,"STAGE0.DAT"))[0];
			auto background=read_grp(read_file(std::filesystem::path(argv[1])/"ST0.GRP"));
			ShotBackend real_shots{*fb,real,{}};ShotBackendBinding real_shot_binding(real_shots);
			PlayerBackend real_player{real_shots,real,real,real,{},{},{}};PlayerBackendBinding real_player_binding(real_player);
			reset({});load_obstacles(layout);obstacles_update_and_render(true);
			require(obstacles.count==2 && obstacles.type[0]==OT_PORTAL && obstacles.type[1]==OT_PORTAL,"Real first-stage portal layout");
			fb->palette=background.palette;
			for(unsigned page=0;page<2;++page) {fb->blit(page,0,0,background.images[0],false);render_layout(*fb,layout,real,page);}
			orb_cur_left=obstacles.left[0];orb_cur_top=obstacles.top[0];orb_prev_left=orb_cur_left;orb_prev_top=orb_cur_top;
			page_access(0);ptn_put_8(orb_cur_left,orb_cur_top,PTN_ORB);
			std::uint64_t trace=14695981039346656037ull;
			for(int frame=1;frame<=40;++frame) {
				obstacles_update_and_render(false);
				for(auto& page:fb->pages) for(auto& plane:page) for(auto value:plane) {trace^=value;trace*=1099511628211ull;}
				if(frame==20 && argc==3) dump_bmp(argv[2],*fb);
			}
			require(!orb_in_portal && orb_cur_left==obstacles.left[1] && orb_cur_top==obstacles.top[1],"Real first-stage portal destination");
			require(trace==0xbbf1ac499dc3129eull,"Real portal framebuffer trace regression");
			std::cout << "Real STAGE0 portal animation trace " << std::hex << trace << std::dec << '\n';
			reset({});
		}
		std::cout << "Original bumper/bar collision, portal RNG/keyframes and all turret groups/timing passed\n";
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}
