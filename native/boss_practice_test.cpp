#include "boss_practice.hpp"
#include "practice_menu.hpp"
#include "stage_session.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "extend_backend.hpp"
#include "boss_hit_backend.hpp"
#include "singyoku_backend.hpp"
#include "card_backend.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <algorithm>
using namespace th01_native;
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 if(argc!=3)throw std::invalid_argument("Usage: boss_practice_test ASSETS TEST_PROFILE");
 std::filesystem::path profile=argv[2];std::filesystem::create_directories(profile);
 std::filesystem::remove(profile/"OTHER.DAT");BossProgress progress(profile);
 for(unsigned mode=0;mode<2;++mode)for(unsigned rank=0;rank<4;++rank)for(unsigned boss=0;boss<7;++boss)require(!progress.unlocked(GameMode(mode),rank,BossId(boss)),"Missing OTHER.DAT starts locked");
 require(progress.defeated(GameMode::challenge,2,BossId::sariel),"Save one unlock");
 BossProgress reopened(profile);
 for(unsigned mode=0;mode<2;++mode)for(unsigned rank=0;rank<4;++rank)for(unsigned boss=0;boss<7;++boss)require(reopened.unlocked(GameMode(mode),rank,BossId(boss))==(mode==1&&rank==2&&boss==5),"Unlock is isolated to mode/rank/boss and persists");
 auto bad=profile/"corrupt";std::filesystem::create_directories(bad);{std::ofstream out(bad/"OTHER.DAT");out<<"bad";}
 BossProgress corrupt(bad);require(!corrupt.error().empty()&&!corrupt.defeated(GameMode::standard,0,BossId::mima),"Corrupt file cannot be overwritten silently");require(read_file(bad/"OTHER.DAT")==Bytes({'b','a','d'}),"Corrupt evidence preserved");
 for(unsigned mode=0;mode<2;++mode)for(unsigned rank=0;rank<4;++rank)for(unsigned boss=0;boss<7;++boss)require(progress.defeated(GameMode(mode),rank,BossId(boss)),"All 56 unlocks writable");
 BossProgress all(profile);for(unsigned mode=0;mode<2;++mode)for(unsigned rank=0;rank<4;++rank)for(unsigned boss=0;boss<7;++boss)require(all.unlocked(GameMode(mode),rank,BossId(boss)),"All 56 unlocks persist");
 auto blocked=profile/"not-a-directory";{std::ofstream out(blocked);out<<"x";}
 BossProgress not_saved(blocked);require(!not_saved.defeated(GameMode::standard,1,BossId::mima),"Unwritable progress is reported without crashing");
 FontRom font(read_file(std::filesystem::path(argv[1])/"FONT.ROM"));Framebuffer background;auto art=read_grp(read_file(std::filesystem::path(argv[1])/"REIIDEN3.GRP"));background.palette=read_grp(read_file(std::filesystem::path(argv[1])/"REIIDEN2.GRP")).palette;background.blit(0,0,0,art.images.at(0),false);
 BossPracticeMenu menu(background,font,all,1);
 auto tap=[&](InputKeys k){menu.step({});menu.step(k);menu.step({});};
 InputKeys confirm;confirm.shot=true;for(unsigned i=0;i<20;++i)menu.step(confirm);require(!menu.ready(),"Held entry confirm does not launch a boss");menu.step({});tap(confirm);tap(confirm);tap(confirm);require(menu.ready()&&menu.selected()==BossId::shingyoku,"Three fresh confirms choose mode/rank/boss");
 menu.resume();InputKeys down;down.down=true;tap(down);require(menu.selected()==BossId::yuugenmagan,"Boss list position survives resume");
 dump_bmp(profile/"boss-list.bmp",menu.framebuffer());
 InputKeys back;back.strike=true;tap(back);tap(back);tap(back);require(menu.cancelled(),"Back traverses rank and mode before leaving practice");
 BossProgress locked(profile/"locked");BossPracticeMenu locked_menu(background,font,locked,1);
 for(unsigned i=0;i<3;++i){locked_menu.step({});locked_menu.step(confirm);locked_menu.step({});}
 require(!locked_menu.ready(),"Locked boss cannot be launched");
 unsigned events=0;SessionHost host{[]{return InputKeys{};},[](const Framebuffer&,unsigned){},{},{},[&](BossId,GameMode,unsigned){++events;}};
 GameOptions opts;opts.credit_bombs=0;opts.credit_lives_extra=0;
 for(unsigned mode=0;mode<2;++mode){opts.mode=GameMode(mode);
  for(unsigned b=0;b<7;++b){auto entry=boss_entries()[b];std::cerr<<"Practice init "<<mode<<" "<<entry.name<<"\n";StageSession session(argv[1],host,1,entry.stage,profile,entry.route,opts,SessionPurpose::boss_practice);
   require(native_extend_lives()==LIVES_MAX&&rem_bombs==BOMBS_MAX,"Practice ignores Option and starts with maximum stocks");require(session.challenge_rules().mode==opts.mode,"Practice preserves selected rules");require(session.stage_number()==entry.stage&&session.selected_route()==entry.route,"Correct direct boss stage/route");
  }
 }
 {
  StageSession session(argv[1],host,1,10,profile,ROUTE_JIGOKU,opts,SessionPurpose::boss_practice);
  std::cerr<<"Challenge miss test\n";for(unsigned i=0;i<10;++i){player_is_hit=true;session.step({});}
  require(session.state()==SessionState::playing&&session.challenge_rules().misses==10,"Challenge practice remains alive beyond stock cap");
 }
 opts.mode=GameMode::standard;
 {
  StageSession session(argv[1],host,1,10,profile,ROUTE_JIGOKU,opts,SessionPurpose::boss_practice);
  std::cerr<<"Standard death test\n";for(unsigned i=0;i<12&&session.state()==SessionState::playing;++i){player_is_hit=true;session.step({});}
  require(session.state()==SessionState::practice_failed,"Standard exhaustion bypasses registration/Continue");session.step({});require(session.state()==SessionState::practice_failed,"Practice failure cannot enter normal score flow");
 }
 {
  std::vector<InputKeys> presses;unsigned index=0;
  SessionHost pause_host{[&](){return index<presses.size()?presses[index++]:InputKeys{};},[](const Framebuffer&,unsigned){}};
  auto clean=profile/"clean-practice";std::filesystem::create_directories(clean);
  StageSession session(argv[1],pause_host,1,10,clean,ROUTE_JIGOKU,opts,SessionPurpose::boss_practice);
  InputKeys accept;accept.shot=true;InputKeys direction;direction.down=true;InputKeys pause_key;pause_key.escape=true;
  presses={InputKeys{},accept};index=0;session.step(pause_key);require(session.state()==SessionState::playing,"Practice pause Resume returns to same boss");
  session.step({});InputKeys cancel;cancel.strike=true;presses={InputKeys{},cancel};index=0;session.step(pause_key);require(session.state()==SessionState::playing,"B in practice pause returns to fight");
  session.step({});presses={InputKeys{},direction,InputKeys{},accept};index=0;session.step(pause_key);
  require(session.state()==SessionState::practice_retry,"Practice pause Retry requests a fresh session");
  require(std::filesystem::is_empty(clean),"Practice creates no original score/option/progress files");
 }
 require(events==0,"Practice never grants normal unlock events");
 // Full defeat/phase paths for all seven bosses in practice, using existing damage hook.
 for(unsigned b=0;b<7;++b){auto entry=boss_entries()[b];StageSession session(argv[1],host,1,entry.stage,profile,entry.route,opts,SessionPurpose::boss_practice);
  for(unsigned warm=0;warm<30;++warm){player_invincible=true;player_invincibility_time=1000;orb_cur_left=orb_prev_left=16;orb_cur_top=orb_prev_top=340;session.step({});}
  unsigned ticks=0;while(session.state()==SessionState::playing&&ticks++<60000){player_invincible=true;player_invincibility_time=1000;
   bool damage=false;
   if(b==0){auto phase=native_singyoku_phase();damage=native_singyoku_hp()>(phase==1?6:0)&&(phase==1||phase==2);}
   if(b==1){auto st=session.yuugenmagan_state();int target=st.phase==1?15:st.phase==3?12:st.phase==5?10:st.phase==7?8:st.phase==9?2:st.phase==13?0:99;damage=st.hp>target;}
   if(b==2){auto st=session.mima_state();damage=(st.phase==1&&st.hp>6)||(st.phase==3&&st.hp>0);}
   if(b==3){auto st=session.elis_state();damage=(st.phase==1&&st.hp>10)||(st.phase==3&&st.hp>6)||(st.phase==5&&st.hp>0);}
   if(b==4){auto st=session.kikuri_state();damage=(st.phase==2&&st.hp>6)||(st.phase==6&&st.hp>0);}
   if(b==5){auto st=session.sariel_state();damage=st.hp>0&&(st.phase<99||(st.phase>=100&&st.frame>=20));}
   if(b==6){auto st=session.konngara_state();int target=st.phase==1?15:st.phase==3?12:st.phase==5?9:st.phase==7?0:99;damage=st.hp>target;}
   test_damage=damage;orb_cur_left=orb_prev_left=0;orb_cur_top=orb_prev_top=380;session.step({});}
  require(session.state()==SessionState::practice_cleared,"Full boss defeat reaches practice result without route/TOTLE/ending");session.step({});require(session.state()==SessionState::practice_cleared,"Clear result remains terminal");if(b==5){const auto before=score;session.present_practice_result();require(score>before&&session.state()==SessionState::practice_cleared,"Final boss displays TOTLE and adds bonus without entering ending");}std::cout<<entry.name<<" practice defeat passed in "<<ticks<<" ticks\n";
 }
 require(events==0,"All seven practice clears leave progress untouched");
 std::cout<<"Boss practice persistence, grouping, resources, input, death and seven full clears passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
