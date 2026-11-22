#include "stage_session.hpp"
#include "card_backend.hpp"
#include "th01/main/stage/stages.hpp"
#include "bonus_values.hpp"
#include "framebuffer_dump.hpp"
#include "singyoku_backend.hpp"
#include "item_runtime.hpp"
#include "extend_backend.hpp"
#include <algorithm>
#include <iostream>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main(int argc,char** argv) {
	try {
		if(argc<2 || argc>3) throw std::invalid_argument("Usage: stage_session_test ASSET_DIR [CLEAR_BMP]");
		auto profile=std::filesystem::absolute(argv[0]).parent_path()/"session-test-profile";
		unsigned waits=0,base_polls=0;SessionHost host{[&] {InputKeys keys;keys.enter=(base_polls++%2)==0;return keys;},[&](const Framebuffer&,unsigned n) {waits+=n;}};
		{
			StageSession session(argv[1],host,1,1,profile);
			require(waits==208,"Start runs six eight-frame glyphs followed by two 35-frame STAGE phases");
			require(session.state()==SessionState::waiting_for_shot && cards.count==26,"Real stage 1 starts waiting with 26 cards");
			auto orb_x=orb_cur_left,orb_y=orb_cur_top,player_x=player_left;
			InputKeys keys;keys.left=true;session.step(keys);
			require(player_left<player_x && orb_cur_left==orb_x && orb_cur_top==orb_y,"Player moves before launch while orb waits");
			keys={};keys.shot=true;session.step(keys);
			require(session.state()==SessionState::playing,"Original shot input launches stage");
			for(unsigned i=0;i<20;++i) session.step({});
			require(orb_cur_left!=orb_x || orb_cur_top!=orb_y,"Composed gameplay advances original orb physics");
			player_is_hit=true;auto before=waits;session.step({});
			require(session.state()==SessionState::playing && !player_is_hit && player_invincibility_time==150 && waits>before+80,"Actual miss animation and life restart are integrated");
			auto sounds=session.take_sound_commands();require(std::find(sounds.begin(),sounds.end(),5)!=sounds.end(),"Miss sound command reaches host");
			require(session.take_sound_commands().empty(),"Sound command drain does not replay old sounds");
			keys={};keys.escape=true;session.step(keys);
			require(session.state()==SessionState::playing,"Actual blocking pause resumes through host input");
			auto score_before=score;auto expected_bonus=stage_bonus_values(stage_timer,cardcombo_max,3,rem_bombs,1).total;
			stage_cleared=true;session.step({});
			require(session.state()==SessionState::stage_cleared,"Stage completion reaches explicit progression boundary");
			require(score==score_before+expected_bonus,"Bonus commits once after real miss life decrement");
			if(argc==3) dump_bmp(argv[2],session.framebuffer());
			before=waits;session.step({});require(waits==before,"Terminal session does not advance gameplay");
			for(unsigned next=2;next<=5;++next) {
				std::vector<unsigned> stock_pixels;for(int y=0;y<32;++y) for(int x=0;x<200;++x) stock_pixels.push_back(session.framebuffer().pixel(0,x,y));
				keys={};keys.shot=true;before=waits;session.step(keys);
				require(session.state()==SessionState::stage_cleared,"Original bonus latch waits for press/release rather than held key");
				session.step({});
				require(session.stage_number()==next,"Acknowledgement advances exactly one stage");
				if(next==5) {
					require(session.state()==SessionState::playing && native_singyoku_phase()==1 && native_singyoku_hp()==8,"Stage 5 starts actual Singyoku after entrance");
					require(waits==before+408 && cards.count==0 && player_left==PLAYER_LEFT_START,"Boss startup runs six-glyph title and entrance, clears regular objects and centers player");
					player_invincibility_time=10000;
					for(unsigned i=0;i<120;++i) session.step({});
					require(session.state()==SessionState::playing && Pellets.native_alive_count()>0,"Boss gameplay loop advances AI and moving pellets");
					keys={};keys.escape=true;session.step(keys);require(session.state()==SessionState::playing,"Pause resumes during boss fight");
					player_is_hit=true;session.step({});require(session.state()==SessionState::playing && !player_is_hit && player_invincibility_time==150 && native_singyoku_hp()==8,"Boss miss respawns player without resetting boss HP");
					player_invincibility_time=10000;
					auto boss_tick=[&]() {orb_cur_left=orb_prev_left=0;orb_cur_top=orb_prev_top=380;session.step({});};
					for(unsigned i=0;i<2;++i) {test_damage=true;boss_tick();}
					for(unsigned i=0;i<1000 && native_singyoku_phase()!=2;++i) boss_tick();
					require(native_singyoku_phase()==2,"Session boss reaches original second phase");
					if(argc==3) dump_bmp(std::string(argv[2])+"-boss.bmp",session.framebuffer());
					while(native_singyoku_hp()>0) {test_damage=true;boss_tick();}
					before=waits;boss_tick();require(session.state()==SessionState::boss_defeated && session.selected_route()==ROUTE_MAKAI && waits>before+500,"Boss defeat runs original fade and confirms default route through host input");
					auto before_totle=score;auto expected_totle=10*(std::min<int>(std::uint16_t(stage_timer*5u),6553)+std::min<int>(cardcombo_max*200,6553)+std::min<int>(native_extend_lives()*500+rem_bombs*200,6553)+5000);
					session.step({});require(session.state()==SessionState::next_scene_pending && score==before_totle+expected_totle,"Post-route flow shows original TOTLE, commits boss bonus once and reaches next-scene boundary");
					auto stable=score;session.step({});require(score==stable && session.stage_number()==6 && session.state()==SessionState::waiting_for_shot,"Finished TOTLE enters route stage 6 without granting its bonus twice");
					break;
				}
				require(session.state()==SessionState::playing && !stage_cleared && !player_is_hit && player_invincibility_time==0,"Next regular stage starts without old hit/invincibility/clear state");
				unsigned pixel=0;for(int y=0;y<32;++y) for(int x=0;x<200;++x) require(session.framebuffer().pixel(0,x,y)==stock_pixels.at(pixel++),"Inter-stage background rebuild preserves life/bomb HUD pixels");
				require(waits==before+70 && session.framebuffer().text_overlay.empty(),"Original two 35-frame title phases finish and clear overlay");
				require(cards.count>0 && orb_cur_left==ORB_LEFT_START && orb_cur_top==ORB_TOP_START && cardcombo_cur==0 && cardcombo_max==0,"Next layout loads and resets orb/combo");
				if(argc==3) dump_bmp(std::string(argv[2])+"-stage"+std::to_string(next)+".bmp",session.framebuffer());
				for(unsigned i=0;i<10;++i) session.step({});
				stage_cleared=true;session.step({});require(session.state()==SessionState::stage_cleared,"Each regular stage reaches its own bonus screen");
			}
		}
		StageSession* active=nullptr;bool decline=false;unsigned menu_polls=0,registration_polls=0;
		SessionHost end_host{[&]() {InputKeys keys;if(active && active->state()==SessionState::continue_prompt) {unsigned n=menu_polls++;if(n>10) throw std::runtime_error("Continue input stalled");if(decline) {keys.down=n==1;keys.enter=n==3;}else keys.enter=n==1;}else {if(registration_polls>100) throw std::runtime_error("Registration input stalled");keys.enter=(registration_polls++%2)==1;}return keys;},host.present_wait};
		{
		StageSession next(argv[1],end_host,1,1,profile);active=&next;require(next.state()==SessionState::waiting_for_shot && !stage_cleared && !player_is_hit,"A new session resets previous terminal state");
		InputKeys launch;launch.shot=true;next.step(launch);
		for(unsigned i=0;i<5;++i) {player_is_hit=true;next.step({});}
		require(next.state()==SessionState::game_over && rem_bombs==1 && next.framebuffer().display_scroll_y==0,"Final life loss runs Game Over animation and resets bomb stock/scroll");
		if(argc==3) dump_bmp(std::string(argv[2])+"-gameover.bmp",next.framebuffer());
		next.step({});require(next.state()==SessionState::waiting_for_shot && next.stage_number()==1 && score==0 && rem_bombs==1 && native_point_value==0 && !player_is_hit,"Continue Yes reloads current regular stage with reset score/point value and saved bomb stock");
		next.step(launch);stage_cleared=true;next.step({});next.step(launch);next.step({});require(next.stage_number()==2,"Reach stage 2 before Continue restart test");
		menu_polls=registration_polls=0;for(unsigned i=0;i<5;++i) {player_is_hit=true;next.step({});}
		next.step({});require(next.stage_number()==2 && next.state()==SessionState::waiting_for_shot && player_left==PLAYER_LEFT_START && cardcombo_cur==0 && score==0,"Continue restarts stage 2, retaining stage identity and resetting player/combo/score");
		decline=true;menu_polls=registration_polls=0;next.step(launch);for(unsigned i=0;i<5;++i) {player_is_hit=true;next.step({});}
		next.step({});require(next.state()==SessionState::title_pending,"Continue No returns to explicit unported title boundary");
		}
		active=nullptr;decline=false;menu_polls=registration_polls=0;
		StageSession boss(argv[1],end_host,1,5,profile);active=&boss;
		for(unsigned i=0;i<5;++i) {player_is_hit=true;boss.step({});}
		require(boss.state()==SessionState::game_over,"Boss final life loss reaches Game Over");boss.step({});
		require(boss.state()==SessionState::playing && boss.stage_number()==5 && native_singyoku_hp()==8 && native_singyoku_phase()==1 && score==0 && rem_bombs==1,"Continue on boss stage reloads Singyoku rather than skipping or returning to stage 1");
		std::cout << "Real stage session startup, movement, launch, miss, pause, clear and restart passed\n";
	} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
