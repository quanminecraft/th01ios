#include "stage_session.hpp"
#include "timer_backend.hpp"
#include "singyoku_backend.hpp"
#include "item_runtime.hpp"
#include "extend_backend.hpp"
#include "timer_backend.hpp"
#include "obstacle_backend.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main(int argc,char** argv) {
	try {
		if(argc<2 || argc>3) throw std::invalid_argument("Usage: route_scene_test ASSET_DIR [BMP_PREFIX]");
		auto archive=load_archive(argv[1]);
		for(route_t route:{ROUTE_MAKAI,ROUTE_JIGOKU}) {
			StageSession* active=nullptr;unsigned route_polls=0,other_polls=0,frames=0;
			SessionHost host{[&]() {InputKeys keys;if(active && active->stage_number()==5 && active->state()==SessionState::playing && native_singyoku_phase()==8) {unsigned n=route_polls++;if(n>10) throw std::runtime_error("Route input stalled");keys.down=route==ROUTE_JIGOKU && n==1;keys.enter=n==3;}else {if(other_polls>1000) throw std::runtime_error("Menu input stalled");keys.enter=(other_polls++%2)==1;}return keys;},[&](const Framebuffer& image,unsigned n) {frames+=n;if(argc==3&&n==50&&active&&active->stage_number()==5&&native_singyoku_phase()==8)dump_bmp(std::string(argv[2])+"-route-choice.bmp",image);}};
			auto profile=std::filesystem::absolute(argv[0]).parent_path()/(route==ROUTE_MAKAI ? "route-profile-makai" : "route-profile-jigoku");
			StageSession session(argv[1],host,1,5,profile);active=&session;player_invincibility_time=10000;
			auto safe_tick=[&]() {orb_cur_left=orb_prev_left=0;orb_cur_top=orb_prev_top=380;session.step({});};
			for(unsigned i=0;i<8;++i) safe_tick();
			for(unsigned i=0;i<2;++i) {test_damage=true;safe_tick();}
			for(unsigned i=0;i<1000 && native_singyoku_phase()!=2;++i) safe_tick();
			require(native_singyoku_phase()==2,"Controlled boss hits enter phase 2");
			while(native_singyoku_hp()>0) {test_damage=true;safe_tick();}
			safe_tick();require(session.state()==SessionState::boss_defeated && session.selected_route()==route,"Actual route menu selects requested route");
			session.step({});require(session.state()==SessionState::next_scene_pending,"Original TOTLE finishes before scene handoff");
			auto score_before=score;auto lives_before=native_extend_lives();auto bombs_before=rem_bombs;
			session.step({});require(session.state()==SessionState::waiting_for_shot && session.stage_number()==6 && score==score_before && native_extend_lives()==lives_before && rem_bombs==bombs_before,"Scene handoff preserves score/stock and starts stage 6 shot gate");
			auto expected=read_scene(asset(archive,route==ROUTE_MAKAI ? "STAGE1.DAT" : "STAGE2.DAT"));
			for(unsigned stage=6;stage<=9;++stage) {
				require(session.stage_number()==stage && cards.count==int(expected[stage-6].cards.size()) && obstacles.count==int(expected[stage-6].obstacles.size()) && stage_timer==1200,"Correct route layout and original timer for each stage");
				for(unsigned i=0;i<expected[stage-6].cards.size();++i) require(cards.left[i]==expected[stage-6].cards[i].left && cards.top[i]==expected[stage-6].cards[i].top && cards.hp[i]==expected[stage-6].cards[i].hp,"Card positions/HP come from chosen route data");
				if(argc==3) dump_bmp(std::string(argv[2])+(route==ROUTE_MAKAI ? "-makai-" : "-jigoku-")+std::to_string(stage)+".bmp",session.framebuffer());
				InputKeys shot;shot.shot=true;if(stage==6) {
					session.step(shot);unsigned deaths=native_extend_lives()+1;for(unsigned i=0;i<deaths;++i) {player_is_hit=true;session.step({});}
					require(session.state()==SessionState::game_over,"Route stage can lose all lives");session.step({});
					require(session.stage_number()==6 && session.selected_route()==route && session.state()==SessionState::waiting_for_shot && cards.count==int(expected[0].cards.size()) && score==0,"Continue reloads stage 6 of the chosen route");session.step(shot);
				}
				player_invincibility_time=10000;for(unsigned i=0;i<20;++i) session.step({});
				require(session.state()==SessionState::playing,"Original actors tick on route stage");stage_cleared=true;session.step({});require(session.state()==SessionState::stage_cleared,"Route clear renders regular-stage bonus");
				session.step(shot);session.step({});
			}
			require(session.stage_number()==10 && session.state()==SessionState::playing,"Stage 10 enters the selected route boss");
            if(route==ROUTE_MAKAI) {require(cards.count==0 && obstacles.count==0 && stage_timer==3000 && session.yuugenmagan_state().hp==16 && session.yuugenmagan_state().phase==0,"Makai stage ten starts live YuugenMagan");}
            if(route==ROUTE_JIGOKU) {require(cards.count==0 && obstacles.count==0 && stage_timer==2000 && session.mima_state().hp==12 && session.mima_state().phase==1,"Mima intro handoff clears stage actors and initializes original boss timer");if(argc==3) dump_bmp(std::string(argv[2])+"-jigoku-10.bmp",session.framebuffer());}
		}
		std::cout<<"Both original routes, stage 6-9 layouts/timers, Continue and stage 10 boundary passed\n";
	} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
