#include "stage_session.hpp"
#include "extend_backend.hpp"
#include "card_backend.hpp"
#include "bonus_screen.hpp"
#include <iostream>
int main(int argc,char** argv) {try {
 using namespace th01_native;
 if(argc!=2)throw std::invalid_argument("Usage: options_session_test ASSET_DIR");
 auto require=[](bool value,const char* message) {if(!value)throw std::runtime_error(message);};
 GameOptions options;options.rank=3;options.credit_bombs=2;options.credit_lives_extra=4;
 for(unsigned stage:{1u,2u,3u,4u}) {
  StageSession session(argv[1],{[](){return InputKeys{};},[](const Framebuffer&,unsigned){}},1,stage,{},ROUTE_MAKAI,options);
  require(session.stage_number()==stage&&session.state()==SessionState::waiting_for_shot,"Main-menu Continue supports each shrine stage");
  require(rank==3&&rem_bombs==2&&native_extend_lives()==6,"Title settings reach original gameplay rank, bombs and lives");
 }
 std::optional<std::uint32_t> challenge_bonus;
 for(auto mode:{GameMode::standard,GameMode::challenge})for(std::int8_t lives=0;lives<=4;++lives) {
  options.mode=mode;options.credit_lives_extra=lives;
  StageSession session(argv[1],{[](){return InputKeys{};},[](const Framebuffer&,unsigned){}},1,1,{},ROUTE_MAKAI,options);
  require(native_extend_lives()==(mode==GameMode::standard?lives+2:4),"Player lives setting applies only in Standard");
  require(rank==3&&rem_bombs==2,"Mode-specific life handling preserves rank and bomb options");
  if(mode==GameMode::challenge) {
   auto bonus=stage_bonus_values(3200,0,native_extend_lives(),rem_bombs,1).total;
   if(challenge_bonus)require(bonus==*challenge_bonus,"Player setting cannot change Challenge stock bonus");
   challenge_bonus=bonus;
   require(session.challenge_rules().displayed_misses()==0,"Player setting cannot grant initial Challenge miss credits");
  }
 }
 options.rank=4;bool rejected=false;
 try {StageSession session(argv[1],{[](){return InputKeys{};},[](const Framebuffer&,unsigned){}},1,1,{},ROUTE_MAKAI,options);}catch(const std::invalid_argument&) {rejected=true;}
 require(rejected,"Invalid rank rejected before indexing original tables");
 std::cout<<"Title options applied to gameplay and Continue stages 1-4 passed\n";
}catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}}
