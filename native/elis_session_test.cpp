#include "stage_session.hpp"
#include "extend_backend.hpp"
#include "timer_backend.hpp"
#include "item_runtime.hpp"
#include "obstacle_backend.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
using namespace th01_native;
static void require(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
int main(int argc,char** argv) {
    try {
        if(argc<2 || argc>3) throw std::invalid_argument("Usage: elis_session_test ASSET_DIR [BMP_PREFIX]");
        unsigned polls=0;SessionHost host{[&]() {if(polls>1000) throw std::runtime_error("Elis menu stalled");InputKeys key;key.enter=(polls++%2)==1;return key;},[](const Framebuffer&,unsigned) {}};
        auto profile=std::filesystem::absolute(argv[0]).parent_path()/"elis-session-profile";StageSession session(argv[1],host,1,15,profile,ROUTE_MAKAI);
        require(session.state()==SessionState::playing && session.elis_state().hp==14 && session.elis_state().phase==1 && stage_timer==4000 && pellet_interlace,"Stage fifteen starts full Elis with original timer/interlace");
        unsigned deaths=native_extend_lives()+1;for(unsigned i=0;i<deaths;++i) {orb_cur_left=orb_prev_left=0;orb_cur_top=orb_prev_top=380;player_is_hit=true;session.step({});}
        require(session.state()==SessionState::game_over,"Elis exhausts lives into Game Over");session.step({});require(session.state()==SessionState::playing && session.stage_number()==15 && session.elis_state().hp==14 && session.elis_state().phase==1 && score==0 && rem_bombs==1,"Continue reloads Elis and resident resources at same stage");
        auto tick=[&]() {orb_cur_left=orb_prev_left=0;orb_cur_top=orb_prev_top=380;player_invincibility_time=20000;player_invincible=true;session.step({});};
        for(unsigned i=0;i<20;++i)tick();while(session.elis_state().hp>10){test_damage=true;tick();}
        unsigned ticks=0;while(session.elis_state().phase!=3){tick();if(++ticks>5000)throw std::runtime_error("Elis phase three stalled");}
        while(session.elis_state().hp>6){test_damage=true;tick();}
        while(session.elis_state().phase!=5){tick();if(++ticks>10000)throw std::runtime_error("Elis final phase stalled");}
        require(session.elis_state().bat,"Live Elis HP thresholds enter final bat phase");
        for(unsigned i=0;i<100;++i)tick();
        if(argc==3)dump_bmp(std::string(argv[2])+"-playing.bmp",session.framebuffer());
        while(session.elis_state().hp>0) {test_damage=true;tick();}require(session.state()==SessionState::boss_defeated && session.elis_state().defeated,"Zero HP runs original Elis defeat animation");
        if(argc==3) dump_bmp(std::string(argv[2])+"-defeated.bmp",session.framebuffer());
        auto before=score;auto expected=10*(std::min<int>(std::uint16_t(stage_timer*5u),6553)+std::min<int>(cardcombo_max*200,6553)+std::min<int>(native_extend_lives()*500+rem_bombs*200,6553)+15000);
        session.step({});require(session.state()==SessionState::next_scene_pending && score==before+expected,"Elis TOTLE awards stage-fifteen bonus exactly once");
        auto score_after=score;auto lives=native_extend_lives();auto bombs=rem_bombs;session.step({});require(session.stage_number()==16 && session.state()==SessionState::waiting_for_shot && score==score_after && native_extend_lives()==lives && rem_bombs==bombs && !pellet_interlace,"Stage sixteen preserves resources and resets fresh MAIN interlace mode");
        auto scene=read_scene(asset(load_archive(argv[1]),"STAGE5.DAT"));
        for(unsigned stage=16;stage<=19;++stage) {
            require(session.stage_number()==stage && cards.count==int(scene[stage-16].cards.size()) && obstacles.count==int(scene[stage-16].obstacles.size()) && stage_timer==(stage==19 ? 600 : 1600),"Final Makai regular stages use correct data and route timers");
            if(argc==3 && stage==16) dump_bmp(std::string(argv[2])+"-16.bmp",session.framebuffer());
            InputKeys shot;shot.shot=true;if(stage==16) session.step(shot);for(unsigned i=0;i<20;++i) tick();stage_cleared=true;session.step({});require(session.state()==SessionState::stage_cleared,"Final regular stage clears");session.step(shot);session.step({});
        }
        require(session.stage_number()==20 && session.state()==SessionState::playing && session.sariel_state().hp==18 && session.sariel_state().phase==1 && stage_timer==6000,"Final Makai scene hands off to live Sariel with original HP and timer");
        std::cout<<"Live Elis Continue, HP phases, defeat/TOTLE and Makai sixteen-nineteen passed\n";
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}