#include "sariel_patterns.hpp"
#include "laser_backend.hpp"
#include "grp.hpp"
#include "assets.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 if(argc<2)throw std::invalid_argument("Usage: sariel_lasers_test ASSET_DIR");
 auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS6_A1.GRP"));auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);auto miko=read_sprites(asset(archive,"MIKO.PTN"));PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding pb(player);
 std::uint64_t trace=1469598103934665603ull;
 for(unsigned difficulty=0;difficulty<4;++difficulty)for(unsigned seed:{1u,3u,12345u}){
  rank=difficulty;player_invincible=true;player_is_hit=false;player_left=300;for(auto& l:shootout_lasers)l={};for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);shots.sound_commands.clear();SarielRandomLasers attack(seed);std::uint32_t rng=seed;int x[10]{},y[10]{};unsigned emitted=0;
  auto random=[&](){rng=rng*0x015a4e35u+1;return (rng>>16)&0x7fff;};
  for(unsigned tick=1;tick<=201;++tick){
   if(tick==50)for(unsigned i=0;i<10;++i){x[i]=120+random()%400;y[i]=100+random()%100;}
   bool fires=tick>=50 && tick<150 && tick%10==9;int target=0;if(fires)target=random()%640;
   require(attack.advance()==(tick==201),"Original pattern ends after frame 200");require(native_random_seed==rng,"Original ordered origin and target RNG draws");
   if(tick<50 || tick==58)require(fb->pages[0]==fb->pages[1],"No early draw and first summoning circle restores the background before firing");
   if(fires){unsigned i=(tick-50)/10;auto& l=shootout_lasers[i];auto angle=iatan2(400-y[i],target-x[i]);const int speeds[]={8,9,9,10};
    require(l.alive && l.age==0 && l.origin_left.v==x[i]*256 && l.origin_y.v==y[i]*256,"Original laser origin and slot");
    require(l.step_x.v==Cos8(angle) && l.step_y.v==Sin8(angle) && l.ray_extend_speed==speeds[difficulty] && l.ray_moveout_speed==speeds[difficulty],"Rank speed truncation and random floor aim");
    require(l.width_cel==4 && l.col==4 && l.moveout_at_age==25 && l.damaging,"Original purple laser width, color and lifetime");
    trace^=std::uint16_t(l.step_x.v);trace*=1099511628211ull;trace^=std::uint16_t(l.step_y.v);trace*=1099511628211ull;++emitted;
   }
   update_shootout_lasers();
  }
  require(emitted==10 && shots.sound_commands.size()==10,"Ten lasers and ten firing sounds per attack");for(auto se:shots.sound_commands)require(se==6,"Laser firing SE6");auto before=fb->pages;require(attack.advance() && attack.frame()==0 && fb->pages==before && native_random_seed==rng,"Completed pattern remains inert");
  for(unsigned tick=0;tick<400;++tick)update_shootout_lasers();for(auto& l:shootout_lasers)require(!l.alive,"All lasers drain after pattern completion");require(!player_is_hit,"Invincibility respected during sustained update");
 }
 std::cout<<"Original Sariel random purple lasers: four ranks, RNG, ten launches, sound and drain passed; trace "<<std::hex<<trace<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
