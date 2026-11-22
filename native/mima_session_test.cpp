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
        if(argc<2 || argc>3) throw std::invalid_argument("Usage: mima_session_test ASSET_DIR [BMP_PREFIX]");
        unsigned polls=0,waits=0;
        SessionHost host{[&]() {if(polls>1000) throw std::runtime_error("Mima session menu stalled");InputKeys key;key.enter=(polls++%2)==1;return key;},[&](const Framebuffer&,unsigned n) {waits+=n;}};
        unsigned clears=0;
        host.boss_defeated=[&](BossId boss,GameMode mode,unsigned difficulty){require(boss==BossId::mima&&mode==GameMode::standard&&difficulty==1,"Normal clear event carries original mode/rank/boss even after Continue");++clears;};
        auto profile=std::filesystem::absolute(argv[0]).parent_path()/"mima-session-profile";
        StageSession session(argv[1],host,1,10,profile,ROUTE_JIGOKU);
        require(session.state()==SessionState::playing && session.stage_number()==10 && session.mima_state().hp==12 && session.mima_state().phase==1 && stage_timer==2000,"Direct stage ten starts full Mima gameplay");
        unsigned deaths=native_extend_lives()+1;for(unsigned i=0;i<deaths;++i) {orb_cur_left=orb_prev_left=0;orb_cur_top=orb_prev_top=380;player_is_hit=true;session.step({});}
        require(session.state()==SessionState::game_over,"Mima stage reaches original Game Over after all lives");session.step({});
        require(session.state()==SessionState::playing && session.stage_number()==10 && session.mima_state().hp==12 && session.mima_state().phase==1 && score==0 && rem_bombs==1,"Continue reloads fresh Mima at the same route/stage");
        auto safe_tick=[&]() {orb_cur_left=orb_prev_left=0;orb_cur_top=orb_prev_top=380;player_invincibility_time=10000;player_invincible=true;session.step({});};
        for(unsigned i=0;i<20;++i) safe_tick();while(session.mima_state().hp>6) {test_damage=true;safe_tick();}
        for(unsigned i=0;i<1000 && session.mima_state().phase!=3;++i) safe_tick();require(session.mima_state().phase==3,"Live regular-frame boss hooks reach Mima late phase");
        while(session.mima_state().hp>0) {test_damage=true;safe_tick();}
        require(session.state()==SessionState::boss_defeated && session.mima_state().defeated,"Live Mima zero HP invokes original defeat animation");require(clears==1,"Continue does not block boss unlock event");
        if(argc==3) dump_bmp(std::string(argv[2])+"-defeated.bmp",session.framebuffer());
        auto before=score;auto expected=10*(std::min<int>(std::uint16_t(stage_timer*5u),6553)+std::min<int>(cardcombo_max*200,6553)+std::min<int>(native_extend_lives()*500+rem_bombs*200,6553)+10000);
        session.step({});require(session.state()==SessionState::next_scene_pending && score==before+expected,"Mima defeat runs original TOTLE and commits stage-ten bonus once");
        auto score_after=score;auto lives=native_extend_lives();auto bombs=rem_bombs;session.step({});
        require(session.stage_number()==11 && session.state()==SessionState::waiting_for_shot && session.selected_route()==ROUTE_JIGOKU && score==score_after && native_extend_lives()==lives && rem_bombs==bombs,"Post-Mima handoff preserves resident resources and starts Jigoku stage eleven");
        auto expected_scene=read_scene(asset(load_archive(argv[1]),"STAGE4.DAT"));
        for(unsigned stage=11;stage<=14;++stage) {
            require(session.stage_number()==stage && cards.count==int(expected_scene[stage-11].cards.size()) && obstacles.count==int(expected_scene[stage-11].obstacles.size()) && stage_timer==1400,"Jigoku stages eleven-fourteen use correct scene and timer");
            if(argc==3 && stage==11) dump_bmp(std::string(argv[2])+"-11.bmp",session.framebuffer());
            InputKeys shot;shot.shot=true;if(stage==11) session.step(shot);for(unsigned i=0;i<20;++i) safe_tick();
            stage_cleared=true;session.step({});require(session.state()==SessionState::stage_cleared,"Next-scene stage can clear");session.step(shot);session.step({});
        }
        require(session.stage_number()==15 && session.state()==SessionState::playing && session.kikuri_state().hp==14 && session.kikuri_state().phase==2,"Next-scene progression enters live Kikuri");
        require(clears==1,"Boss transition does not duplicate unlock event");
        std::cout<<"Live Mima gameplay, Game Over/Continue, defeat/TOTLE and Jigoku stages eleven-fourteen passed\n";
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}