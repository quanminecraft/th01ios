#include "stage_session.hpp"
#include "extend_backend.hpp"
#include "timer_backend.hpp"
#include "item_runtime.hpp"
#include "obstacle_backend.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
using namespace th01_native;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
 try{
    if(argc<2)throw std::invalid_argument("Usage: yuugenmagan_session_test ASSET_DIR [BMP_PREFIX]");
    unsigned polls=0,waits=0;
    SessionHost host{[&](){if(polls>1000)throw std::runtime_error("YuugenMagan menu stalled");InputKeys key;key.enter=(polls++%2)==1;return key;},[&](const Framebuffer&,unsigned n){waits+=n;}};
    auto profile=std::filesystem::absolute(argv[0]).parent_path()/"yuugenmagan-session-profile";
    StageSession session(argv[1],host,1,10,profile,ROUTE_MAKAI);
    require(session.state()==SessionState::playing && session.stage_number()==10 && session.yuugenmagan_state().hp==16 && session.yuugenmagan_state().phase==0 && stage_timer==3000,"Direct Makai stage ten starts live YuugenMagan and original timer");
    unsigned deaths=native_extend_lives()+1;
    for(unsigned i=0;i<deaths;++i){orb_cur_left=orb_prev_left=0;orb_cur_top=orb_prev_top=380;player_is_hit=true;session.step({});}
    require(session.state()==SessionState::game_over,"YuugenMagan supports normal Game Over");session.step({});
    require(session.state()==SessionState::playing && session.yuugenmagan_state().hp==16 && session.yuugenmagan_state().phase==0 && score==0 && rem_bombs==1,"Continue reloads fresh YuugenMagan on the same route");
    auto tick=[&](){orb_cur_left=orb_prev_left=0;orb_cur_top=orb_prev_top=380;player_invincibility_time=10000;player_invincible=true;session.step({});};
    unsigned ticks=0;
    auto reach=[&](int phase){while(session.yuugenmagan_state().phase!=phase){tick();if(++ticks>10000)throw std::runtime_error("Session boss phase stalled");}};
    reach(1);const int phases[]={1,3,5,7,9};const int hp[]={15,12,10,8,2};
    for(unsigned i=0;i<5;++i){reach(phases[i]);while(session.yuugenmagan_state().hp>hp[i]){test_damage=true;tick();}}
    reach(13);for(unsigned i=0;i<160;++i)tick();
    if(argc>=3)dump_bmp(std::string(argv[2])+"-playing.bmp",session.framebuffer());
    while(session.yuugenmagan_state().hp>0){test_damage=true;tick();}
    require(session.state()==SessionState::boss_defeated && session.yuugenmagan_state().defeated,"Regular-frame boss hook invokes original defeat animation");
    if(argc>=3)dump_bmp(std::string(argv[2])+"-defeated.bmp",session.framebuffer());
    auto before=score;auto expected=10*(std::min<int>(std::uint16_t(stage_timer*5u),6553)+std::min<int>(cardcombo_max*200,6553)+std::min<int>(native_extend_lives()*500+rem_bombs*200,6553)+10000);
    session.step({});require(session.state()==SessionState::next_scene_pending && score==before+expected,"YuugenMagan defeat commits original stage-ten TOTLE once");
    auto after=score;auto lives=native_extend_lives();auto bombs=rem_bombs;session.step({});
    require(session.stage_number()==11 && session.state()==SessionState::waiting_for_shot && session.selected_route()==ROUTE_MAKAI && score==after && native_extend_lives()==lives && rem_bombs==bombs,"Post-boss Makai scene preserves resident resources");
    auto archive=load_archive(argv[1]);auto scene=read_scene(asset(archive,"STAGE3.DAT"));
    for(unsigned stage=11;stage<=14;++stage){
        require(session.stage_number()==stage && cards.count==int(scene[stage-11].cards.size()) && obstacles.count==int(scene[stage-11].obstacles.size()) && stage_timer==1400,"Makai regular stages eleven-fourteen load original layouts/timer");
        if(argc>=3 && stage==11)dump_bmp(std::string(argv[2])+"-11.bmp",session.framebuffer());
        InputKeys shot;shot.shot=true;if(stage==11)session.step(shot);for(unsigned i=0;i<20;++i)tick();
        stage_cleared=true;session.step({});require(session.state()==SessionState::stage_cleared,"Makai regular-stage clear is playable");session.step(shot);session.step({});
    }
    require(session.stage_number()==15 && session.state()==SessionState::playing && session.elis_state().hp==14 && session.elis_state().phase==1,"Makai stage fourteen transitions into live Elis");
    std::cout<<"Live YuugenMagan, Game Over/Continue, defeat/TOTLE and Makai stages eleven-fourteen passed\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}