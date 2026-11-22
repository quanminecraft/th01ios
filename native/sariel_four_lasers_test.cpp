#include "sariel_patterns.hpp"
#include "laser_backend.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 if(argc<2)throw std::invalid_argument("Usage: sariel_four_lasers_test ASSET_DIR [BMP]");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS6_A3.GRP"));auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);auto miko=read_sprites(asset(archive,"MIKO.PTN"));PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding pb(player);std::uint64_t trace=1469598103934665603ull;
 for(unsigned difficulty=0;difficulty<4;++difficulty)for(int start:{40,304,560}){
  for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);fb->palette=bg.palette;page_access(0);for(auto& l:shootout_lasers)l={};player_invincible=true;player_is_hit=false;rank=difficulty;shots.sound_commands.clear();SarielPattern attack(123,SarielPatternKind::FourAimedLasers);unsigned emitted=0;
  for(unsigned tick=1;tick<=301;++tick){
   player_left=tick<75 ? start:tick<175 ? 608-start:start/2;require(attack.advance()==(tick==301),"Four aimed lasers finish at frame 301");require(native_random_seed==123,"Four aimed lasers consume no RNG");
   bool fires=tick==50 || tick==100 || tick==150 || tick==200;
   if(fires){unsigned slot=tick/50-1;const int x[]={460,180,608,24},y[]={120,120,164,164};int target=slot==0 ? start:slot==1 ? 639-start:slot==2 ? 608-start:start/2;auto angle=iatan2(400-y[slot],target-x[slot]);auto& l=shootout_lasers[slot];const int speeds[]={7,8,8,9};require(l.alive && l.age==0 && l.origin_left.v==x[slot]*256 && l.origin_y.v==y[slot]*256,"Original four laser origins and assigned slots");require(l.step_x.v==Cos8(angle) && l.step_y.v==Sin8(angle),"First target capture, mirrored second aim and live third/fourth aims");require(l.ray_extend_speed==speeds[difficulty] && l.ray_moveout_speed==speeds[difficulty] && l.col==7 && l.width_cel==7 && l.moveout_at_age==50 && l.damaging,"Original rank speed truncation, maximum width and lifetime");trace^=std::uint16_t(l.step_x.v);trace*=1099511628211ull;trace^=std::uint16_t(l.step_y.v);trace*=1099511628211ull;++emitted;}
   require(shots.sound_commands.size()==emitted,"SE6 fires only at four launch keyframes");update_shootout_lasers();
   if(argc>2 && difficulty==1 && start==304 && tick==170)dump_bmp(argv[2],*fb);
  }
  require(emitted==4 && !player_is_hit,"Exactly four lasers, invincible player remains safe");for(auto s:shots.sound_commands)require(s==6,"Original laser SE6");auto before=fb->pages;require(attack.advance() && attack.frame()==0 && before==fb->pages,"Completed attack is inert");for(unsigned tick=0;tick<400;++tick)update_shootout_lasers();for(auto& l:shootout_lasers)require(!l.alive,"All active lasers leave the screen after completion");
 }
 std::cout<<"Original Sariel four aimed lasers: four ranks, moving player targets, width/speed, sounds and drain passed; trace "<<std::hex<<trace<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
