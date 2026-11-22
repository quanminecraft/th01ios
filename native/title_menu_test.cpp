#include "title_menu.hpp"
#include "boss_practice.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include "controller_input.hpp"
#include <cctype>
#include <iostream>
#include <map>
int main(int argc,char** argv) {try {
 using namespace th01_native;
 if(argc<3||argc>4)throw std::invalid_argument("Usage: title_menu_test ASSET_DIR PROFILE [BMP]");
 std::filesystem::path dir=argv[1],profile=argv[2];std::filesystem::create_directories(profile);
 {std::ofstream out(profile/"reiiden.cfg",std::ios::binary|std::ios::trunc);out<<"invalid";}
 FontRom font(read_file(dir/"FONT.ROM"));std::map<std::string,SpriteSet> cache;
 unsigned frames=0,milliseconds=0;std::vector<std::string> audio;
 TitleHost host;
 host.load=[&](const char* filename) {std::string name=filename;for(auto& c:name)c=std::toupper(static_cast<unsigned char>(c));if(!cache.count(name))cache.emplace(name,read_grp(read_file(dir/name)));return cache.at(name);};
 host.present_wait=[&](const Framebuffer&,unsigned n) {frames+=n;};host.delay_ms=[&](unsigned n) {milliseconds+=n;};
 host.audio=[&](const char* op,const char* name) {audio.push_back(std::string(op)+":"+name);};
 auto require=[](bool value,const char* reason) {if(!value)throw std::runtime_error(reason);};
 BossProgress completed(profile);if(!completed.defeated(GameMode::standard,1,BossId::konngara))throw std::runtime_error("Cannot initialize unlocked title fixture");
 TitleMenu title(font,profile,host);
 require(frames==216&&milliseconds==160,"Original title fade/hold/white-line timing");
 require(title.options().rank==1&&title.options().credit_bombs==1,"Malformed config uses original defaults");
 InputKeys key;key.enter=true;title.step(key);
 for(unsigned i=0;i<30;++i)require(title.step(key)==TitleAction::menu&&title.selection()==0,"Held PRESS KEY confirmation must not activate Start");
 title.step({});
 require(title.menu_id()==0&&title.selection()==0,"Hit key enters original main menu");
 if(argc==4)dump_bmp(argv[3],title.framebuffer());
 auto tap=[&](char code) {InputKeys k;if(code=='d')k.down=true;if(code=='u')k.up=true;if(code=='r')k.right=true;if(code=='l')k.left=true;if(code=='e')k.enter=true;if(code=='x')k.escape=true;auto result=title.step(k);title.step({});return result;};
 tap('d');tap('d');tap('d');tap('e');title.step({});require(title.menu_id()==1&&title.selection()==0,"Option entry");
 tap('r');require(title.options().rank==2,"Rank setting applies");
 key={};key.right=true;title.step(key);title.step(key);require(title.options().rank==3,"Held horizontal input changes setting once");title.step({});tap('r');require(title.options().rank==0,"Rank wraps Lunatic to Easy");tap('r');tap('r');
 tap('d');tap('r');require(title.options().bgm_mode==0,"BGM mode setting");
 require(audio.back()=="bgm_stop:","BGM Off silences the current title immediately");
 auto audio_before=audio.size();tap('r');
 require(title.options().bgm_mode==1&&audio.size()==audio_before+3&&audio[audio_before]=="bgm_stop:"&&audio[audio_before+1]=="bgm_load:reimu.mdt"&&audio.back()=="bgm_play:","BGM Off then On resumes title immediately without starting gameplay");
 title.step({});require(audio.size()==audio_before+3,"Idle option frame must not restart BGM");
 tap('l');require(title.options().bgm_mode==0&&audio.back()=="bgm_stop:","Repeated BGM toggle still stops correctly");
 tap('d');tap('r');require(title.options().credit_lives_extra==3,"Player setting changes");
 tap('d');tap('e');title.step({});require(title.menu_id()==2,"Music Test entry");
 tap('l');tap('e');require(audio.back()=="bgm_play:"&&audio[audio.size()-2]=="bgm_load:iris.mdt","Music test wraps to track 14 and dispatches original asset");
 tap('x');title.step({});require(title.menu_id()==1,"Music Test returns to options");
 for(unsigned i=0;i<4;++i)tap('d');tap('e');title.step({});require(title.menu_id()==0&&title.selection()==3,"Options return to main with Option selected");
 tap('u');tap('u');tap('u');auto before_start_audio=audio.size();require(tap('e')==TitleAction::new_game,"Start dispatches new game");
 require(audio.size()==before_start_audio,"Opening mode selection does not fade or restart title music");
 auto menu_pages=std::make_unique<decltype(title.framebuffer().pages)>(title.framebuffer().pages);
 auto before_resume_frames=frames,before_resume_ms=milliseconds;title.resume_menu();
 require(frames==before_resume_frames&&milliseconds==before_resume_ms&&title.framebuffer().pages==*menu_pages&&title.selection()==0&&title.menu_id()==0,"Mode cancel restores existing Start menu without replaying intro or changing selection");
 InputKeys back;back.strike=true;for(unsigned i=0;i<20;++i)require(title.step(back)==TitleAction::menu&&title.selection()==0,"Held B from mode cancel must not activate another menu action");
 title.step({});tap('d');require(title.selection()==1,"Main menu responds after cancelled-mode B is released");
 require(tap('e')==TitleAction::continue_game,"Continue remains usable after mode cancel");
 title.resume_menu();title.step({});tap('d');require(tap('e')==TitleAction::boss_practice,"Main menu dispatches Boss Prac separately from Start and Option");title.resume_menu();title.step({});
 auto bytes=read_file(profile/"reiiden.cfg");require(bytes==Bytes({'R','E','I','I','D','E','N',2,0,1,3}),"Original packed configuration bytes saved");
 audio.clear();TitleMenu reopened(font,profile,host);require(reopened.options().rank==2&&reopened.options().bgm_mode==0&&reopened.options().credit_lives_extra==3,"Config survives title recreation");
 require(std::find(audio.begin(),audio.end(),"bgm_stop:")!=audio.end()&&std::find(audio.begin(),audio.end(),"bgm_play:")==audio.end(),"Returning title honors BGM Off instead of replaying intro music");
 key={};key.enter=true;reopened.step(key);reopened.step({});key={};key.escape=true;require(reopened.step(key)==TitleAction::quit,"Main menu Escape quits");
 for(unsigned kind=0;kind<4;++kind) {
  TitleMenu held(font,profile,host);InputKeys dismiss;
  dismiss.enter=kind==0||kind==2;dismiss.shot=kind==1||kind==2;dismiss.escape=kind==3;
  held.step(dismiss);
  for(unsigned i=0;i<30;++i)require(held.step(dismiss)==TitleAction::menu&&held.selection()==0,"Enter, Z, controller A and Escape consume the title-dismiss press");
  held.step({});InputKeys down;down.down=true;held.step(down);held.step({});
  require(held.selection()==1,"Fresh D-pad press chooses Continue after PRESS KEY release");
  InputKeys confirm;confirm.enter=true;require(held.step(confirm)==TitleAction::continue_game,"Only a fresh confirmation activates the chosen menu item");
 }
 {
  TitleMenu controlled(font,profile,host);InputKeys start;start.escape=true;
  apply_start_button(start,true,true);
  require(start.enter&&!start.escape&&!start.shot,"Physical Start confirms menus, clearing its legacy Esc alias without creating Z+Enter");
  controlled.step(start);for(unsigned i=0;i<10;++i)require(controlled.step(start)==TitleAction::menu,"Held Start only dismisses HIT KEY");
  controlled.step({});require(controlled.step(start)==TitleAction::new_game,"Physical Start chooses Start without quitting the application");
  controlled.resume_menu();controlled.step({});InputKeys down;down.down=true;
  controlled.step(down);controlled.step({});controlled.step(down);controlled.step({});controlled.step(down);controlled.step({});
  require(controlled.selection()==3&&controlled.step(start)==TitleAction::menu,"Physical Start selects Option without quitting");
  controlled.step({});require(controlled.menu_id()==1,"Physical Start opens Options");
  InputKeys gameplay;apply_start_button(gameplay,true,false);
  require(gameplay.escape&&!gameplay.enter&&!gameplay.shot,"Physical Start still pauses gameplay without simultaneous confirm/shot");
 }
 {
  TitleMenu single(font,profile,host);
  // GPTOKEYB F1 can arrive after the same physical SDL controller press.
  // Late controller release/bounce must not create another confirmation.
  for(auto state:std::array<std::pair<bool,bool>,8>{{{false,true},{true,true},{true,false},{true,true},{false,true},{false,false},{false,true},{false,false}}}) {
   InputKeys keys;apply_start_button(keys,start_button_down(state.first,state.second,true),true);
   require(single.step(keys)==TitleAction::menu&&single.selection()==0,"One physical Start through two delayed sources must only dismiss PRESS KEY");
  }
  InputKeys fresh;apply_start_button(fresh,start_button_down(true,true,true),true);
  require(single.step(fresh)==TitleAction::new_game,"A second fresh mapped Start confirms the main-menu choice");
  require(start_button_down(false,true,false)&&!start_button_down(false,true,true),"Desktop native controller remains supported; PortMaster uses only its mapped Start");
 }
 std::cout<<"Original title, options, held input, Music Test, packed config, Start and Quit passed\n";
}catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}}
