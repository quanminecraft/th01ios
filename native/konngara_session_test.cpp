#include "stage_session.hpp"
#include "scoredat_backend.hpp"
#include "extend_backend.hpp"
#include "timer_backend.hpp"
#include "item_runtime.hpp"
#include <iostream>
using namespace th01_native;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
    try{
        if(argc!=2)throw std::invalid_argument("Usage: konngara_session_test ASSET_DIR");
        bool checking_ending=false;unsigned ending_fades=0;unsigned polls=0;SessionHost host{[&](){if(polls>1000)throw std::runtime_error("Menu stalled");InputKeys k;k.enter=(polls++%2)==1;return k;},[&](const Framebuffer& fb,unsigned n){if(checking_ending && n==6){++ending_fades;require(fb.pixel(0,0,0)==0,"Ending outside centered picture must not retain boss background");}}};
        auto profile=std::filesystem::absolute(argv[0]).parent_path()/"konngara-session-profile";StageSession session(argv[1],host,1,20,profile,ROUTE_JIGOKU);
        require(session.state()==SessionState::playing && session.konngara_state().hp==18 && session.konngara_state().phase==1 && stage_timer==8000,"Live stage20 full Konngara starts with original HP/timer");
        auto sounds=session.take_sound_commands();require(std::find(sounds.begin(),sounds.end(),9)!=sounds.end() && session.take_sound_commands().empty(),"Entrance sound commands can be drained once");
        auto tick=[&](){orb_cur_left=orb_prev_left=0;orb_cur_top=orb_prev_top=380;player_invincibility_time=20000;player_invincible=true;session.step({});};
        for(unsigned i=0;i<20;++i)tick();test_damage=true;tick();require(session.konngara_state().hp==17,"Live boss hook applies debug test damage");
        player_is_hit=true;session.step({});require(session.state()==SessionState::playing && session.konngara_state().hp==17,"Normal miss retains Konngara HP");
        unsigned deaths=native_extend_lives()+1;for(unsigned i=0;i<deaths;++i){orb_cur_left=orb_prev_left=0;orb_cur_top=orb_prev_top=380;player_is_hit=true;session.step({});}
        require(session.state()==SessionState::game_over,"Konngara lives exhaust into Game Over");session.step({});require(session.state()==SessionState::playing && session.stage_number()==20 && session.konngara_state().hp==18 && session.konngara_state().phase==1 && score==0 && rem_bombs==1,"Continue reloads fresh Konngara with resident reset");
        auto damage_to=[&](int hp){while(session.konngara_state().hp>hp){test_damage=true;tick();}};
        auto wait_phase=[&](int phase){unsigned n=0;while(session.konngara_state().phase!=phase){tick();if(++n>3000)throw std::runtime_error("Live phase transition stalled");}};
        for(unsigned i=0;i<20;++i)tick();damage_to(15);wait_phase(3);damage_to(12);wait_phase(5);damage_to(9);wait_phase(7);damage_to(1);
        auto before=score;test_damage=true;tick();require(session.state()==SessionState::ending_pending && session.konngara_state().cleared && score==before+50000,"Konngara defeat awards only original clear bonus and hands off to ending boundary");
        auto cleared_score=score;checking_ending=true;session.step({});require(ending_fades>0,"Live ending uses original picture fade timings");require(session.state()==SessionState::title_pending && score==0,"Konngara ending, verdict and registration return to title boundary and clear resident score");require(scoredat_load()==0,"Ending score file reloads");bool stored_clear=false;for(unsigned place=0;place<10;++place)stored_clear|=scoredat_score[place]==cleared_score && scoredat_stages[place]==60 && scoredat_routes[place]==44682;scoredat_free();require(stored_clear,"Ending registration persists final score and cleared route");session.step({});require(session.state()==SessionState::title_pending && score==0,"Completed ending does not replay defeat or award bonus again");
        std::cout<<"Live Konngara miss/Continue, phases, defeat score and ending handoff passed\n";
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
