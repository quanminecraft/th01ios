#include "stage_session.hpp"
#include "extend_backend.hpp"
#include "mode_menu.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "ending_playback.hpp"
#include "title_menu.hpp"
#include "audio_mix.hpp"
#include "framebuffer_dump.hpp"
#include "challenge_hud.hpp"
#include "bomb_backend.hpp"
#include <iostream>
using namespace th01_native;
static void require(bool v,const char* m){if(!v)throw std::runtime_error(m);}
int main(int argc,char** argv){try {
 if(argc!=2)throw std::invalid_argument("Usage: user_regressions_test ASSETS");
 auto root=std::filesystem::path(argv[1]);FontRom font(read_file(root/"FONT.ROM"));
 std::int16_t mono[]={30000,0,0,-30000,32767,32767,-32768,-32768};
 stereo_to_mono(mono,4);
 require(mono[0]==15000&&mono[1]==15000&&mono[2]==-15000&&mono[3]==-15000&&mono[4]==32767&&mono[7]==-32768,"Mono preserves left/right parts without overflow");
 auto blank=std::make_unique<Framebuffer>();auto backdrop=read_grp(read_file(root/"REIIDEN2.GRP"));blank->palette=backdrop.palette;blank->blit(0,0,0,backdrop.images.at(0),false);ModeMenu menu(*blank,font);
 {
  ChallengeHud miss_hud;blank->restore(0,1,0,0,640,400);miss_hud.capture(*blank);
  auto original=blank->pages;
  miss_hud.draw(*blank,font,-12);
  require(blank->status_overlay.pixels.empty(),"Challenge counter uses native HUD, no presentation overlay");
  for(int y=0;y<16;++y)for(int x=128;x<224;++x)
   require(blank->pixel(0,x,y)==blank->pixel(1,x,y),"Miss field matches on both graphics pages");
  // Spaces outside glyphs must retain detailed source pixels, not a black box.
  for(unsigned page=0;page<2;++page)for(int y=0;y<16;++y)for(int x=216;x<224;++x) {
   Framebuffer reference;reference.pages=original;
   require(blank->pixel(page,x,y)==reference.pixel(page,x,y),"Miss field preserves original background outside glyphs");
  }
  auto first=blank->pages;
  miss_hud.draw(*blank,font,1234567890);miss_hud.draw(*blank,font,-12);
  require(blank->pages==first,"Shorter signed miss value clears all old digits");
  blank->palette.fill(0xff000000u);auto black=blank->rgba(0);
  require(std::all_of(black.begin(),black.end(),[](auto p){return p==0xff000000u;}),"Native miss HUD follows palette fade");

 }
 InputKeys confirm;confirm.shot=true;
 for(unsigned i=0;i<30;++i)require(!menu.step(confirm),"Start press must not select a game mode");
 menu.step({});InputKeys down;down.down=true;menu.step(down);menu.step({});
 require(menu.mode()==GameMode::challenge&&menu.step(confirm),"Fresh direction/A chooses Challenge");
 ChallengeRules rules;rules.mode=GameMode::challenge;rules.extend();rules.extend();
 require(rules.displayed_misses()==-2&&rules.good_ending(),"Extend credits allow negative MISS");
 for(unsigned i=0;i<13;++i)rules.miss();require(!rules.good_ending(),"Adjusted MISS 11 gives bad ending");
 rules.extend();require(rules.good_ending()&&rules.displayed_misses()==10,"Adjusted MISS 10 allows good ending");
 unsigned waits=0,black_holds=0,polls=0,last_entrance=0;std::string track;
 SessionHost host;
 host.poll=[&](){if(++polls>200)throw std::runtime_error("Unexpected blocking input menu");return InputKeys{};};
 host.present_wait=[&](const Framebuffer& fb,unsigned n){
  if(n==90){auto pixels=fb.rgba(0);require(std::all_of(pixels.begin(),pixels.end(),[](auto p){return p==0xff000000u;}),"Preparation hold must be fully black");++black_holds;}
  waits+=n;
 };
 host.sound=[&](unsigned id){if(id==14)last_entrance=waits;};
 host.audio=[&](const GameAudioCommand& command){if(command.operation=="bgm_load")track=command.asset;if(command.operation=="bgm_play"&&track!="INIT.MDT")require(waits>=last_entrance+8,"BGM must start after entrance SE frames");};
 GameOptions options;options.mode=GameMode::challenge;
 auto profile=std::filesystem::absolute(argv[0]).parent_path()/"challenge-test-profile";
 std::filesystem::create_directories(profile);dump_bmp(profile/"mode.bmp",menu.framebuffer());
 {
  StageSession session(root,host,1,1,profile,ROUTE_MAKAI,options);
  require(waits==208&&black_holds==1,"Stage 1 has 90 black frames before the 118-frame entrance");
  InputKeys pause;pause.escape=true;session.step(pause);session.step({});session.step(confirm);session.step({});
  require(session.state()==SessionState::playing&&!paused,"Pre-launch pause is discarded rather than deferred");
  for(unsigned i=0;i<11;++i){player_is_hit=true;session.step({});require(session.state()==SessionState::playing,"Challenge never enters Game Over when lives run out");}
  require(session.challenge_rules().misses==11&&!session.challenge_rules().good_ending(),"Real misses update Challenge ending eligibility");
  score=SCORE_PER_EXTEND;score_extend_update_and_render();
  require(session.challenge_rules().displayed_misses()==10&&session.challenge_rules().good_ending(),"Real original extend hook credits one miss");
  score_extend_update_and_render();require(session.challenge_rules().displayed_misses()==10,"Same score threshold is not credited twice");
  session.step({});dump_bmp(profile/"challenge-hud.bmp",session.framebuffer());
  // Exhausted stock can cross into a boss reload, which clears HUD captures.
  // Restoring a fictitious refilled life icon there used to throw/crash.
  score=0;extend_next=1;
  for(unsigned next=2;next<=5;++next) {
   native_extend_lives()=0;stage_cleared=true;session.step({});session.step({});session.step(confirm);session.step({});
   require(session.stage_number()==next,"Challenge can advance with exhausted underlying stock");
  }
  native_extend_lives()=0;player_is_hit=true;session.step({});
  require(session.state()==SessionState::playing&&session.challenge_rules().misses==12,"Challenge respawns after empty-stock boss HUD reload without restoring uncaptured life icons");
 }
 // Clearing the last card during a bomb must not retain inverted HUD pixels.
 for(auto mode:{GameMode::standard,GameMode::challenge})for(unsigned clear_frame:{42u,64u,88u,136u,140u}) {
  options.mode=mode;polls=0;
  StageSession session(root,host,1,1,profile,ROUTE_MAKAI,options);session.step(confirm);session.step({});
  for(unsigned tick=0;tick<=clear_frame;++tick)native_bomb_update(tick);
  stage_cleared=true;session.step({});
  require(session.framebuffer().display_scroll_y==0,"Interrupted bomb resets screen shake at stage clear");
  require(!bomb_damaging,"Stage clear stops bomb damage before bonus presentation");
  const auto& picture=session.framebuffer();
  for(int y=0;y<32;++y)for(int x=0;x<128;++x)
   require(picture.pixel(0,x,y)==picture.pixel(1,x,y),"Stage clear during bomb restores original HUD border instead of retaining inversion");
  session.step({});session.step(confirm);session.step({});
  require(session.stage_number()==2&&session.framebuffer().display_scroll_y==0,"Next stage does not inherit bomb shake");
  const auto& next=session.framebuffer();
  for(int y=0;y<32;++y)for(int x=0;x<128;++x)
   require(next.pixel(0,x,y)==next.pixel(1,x,y),"Restored HUD survives next stage using the original backing page");
 }
 // All reload stages, both routes: black preparation precedes title/audio.
 for(auto route:{ROUTE_MAKAI,ROUTE_JIGOKU})for(unsigned stage:{5u,6u,10u,11u,15u,16u,20u}){
  waits=black_holds=polls=last_entrance=0;track.clear();
  StageSession session(root,host,1,stage,profile,route);
  require(black_holds==1,"Each scene/boss reload has exactly one preparation hold");
  require(last_entrance>=90,"Scene entrance SE follows the black preparation hold");
 }
 options.mode=GameMode::standard;polls=0;
 {
  StageSession session(root,host,1,1,profile,ROUTE_MAKAI,options);session.step(confirm);
  native_extend_lives()=0;player_is_hit=true;session.step({});require(session.state()==SessionState::game_over,"Standard retains original finite lives");
 }
 struct EndingBranch {bool good;};
 for(bool makai:{false,true})for(bool good:{false,true}) {
  EndingResult result;result.makai=makai;result.continues=good?42:0;result.good_ending_override=good;
  auto fb=std::make_unique<Framebuffer>();VerdictHost end;
  end.load=[&](const char* file) {
   std::string name=file;for(auto& c:name)c=std::toupper(static_cast<unsigned char>(c));
   if(name=="ED2A.GRP")throw EndingBranch{false};if(name=="ED3A.GRP")throw EndingBranch{true};
   return read_grp(read_file(root/name));
  };
  end.wait=[](unsigned){};end.text=[](int,int,int,const char*){};end.input=[](bool){return std::array<bool,2>{false,false};};
  end.random=[](){return 0u;};end.registration_colors=[](){};end.registration=[](std::uint32_t,int,std::uint16_t){};end.resident_clear=[](){};
  bool branch_seen=false;
  try {play_ending(*fb,result,std::move(end),[](const char*,const char*){});}catch(EndingBranch branch){require(branch.good==good,"Challenge selects actual good/bad ending independently of Continue count");branch_seen=true;}
  require(branch_seen,"Ending must select a good/bad branch");
 }
 TitleHost title_host;
 title_host.load=[&](const char* file){std::string name=file;for(auto& c:name)c=std::toupper(static_cast<unsigned char>(c));return read_grp(read_file(root/name));};
 title_host.present_wait=[](const Framebuffer&,unsigned){};title_host.delay_ms=[](unsigned){};title_host.audio=[](const char*,const char*){};
 GameOptions remembered;
 {
  TitleMenu title(font,profile,title_host);title.step(confirm);title.step({});
  auto tap=[&](char key){InputKeys input;input.down=key=='d';input.right=key=='r';input.escape=key=='b';input.shot=key=='a';title.step(input);title.step({});};
  tap('d');tap('d');tap('a');tap('r');tap('d');tap('d');tap('r');
  remembered=title.options();auto saved=read_file(profile/"reiiden.cfg");
  require(saved[7]==remembered.rank&&saved[10]==remembered.credit_lives_extra,"Options persist immediately before starting a run");
 }
 polls=0;
 {StageSession session(root,host,1,1,profile,ROUTE_MAKAI,remembered);require(rank==remembered.rank,"Selected rank reaches gameplay");}
 {TitleMenu returned(font,profile,title_host,remembered);require(returned.options().rank==remembered.rank&&returned.options().credit_lives_extra==remembered.credit_lives_extra,"Gameplay return keeps edited options");}
 {TitleMenu reopened(font,profile,title_host);require(reopened.options().rank==remembered.rank&&reopened.options().credit_lives_extra==remembered.credit_lives_extra,"Edited options also survive a fresh launch");}
 std::cout<<"Mode selection, Challenge resources/ending threshold, black reloads, audio order, pre-launch input and Standard passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
