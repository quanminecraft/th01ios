#include "sariel_patterns.hpp"
#include "laser_backend.hpp"
#include "pellet_backend.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 if(argc<2)throw std::invalid_argument("Usage: sariel_radial_rain_test ASSET_DIR [BMP]");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS6_A3.GRP"));auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);auto miko=read_sprites(asset(archive,"MIKO.PTN"));PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding pb(player);std::uint64_t trace=1469598103934665603ull;
 for(unsigned difficulty=0;difficulty<4;++difficulty)for(unsigned seed:{1u,123u})for(auto kind:{SarielPatternKind::FourLasersWithRain,SarielPatternKind::RadialStacksAndLasers}){
  for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);fb->palette=bg.palette;page_access(0);for(auto& l:shootout_lasers)l={};rank=difficulty;native_pellet_speed=0;player_invincible=true;player_is_hit=false;player_left=304;shots.sound_commands.clear();SarielPattern attack(seed,kind);std::uint32_t rng=seed;auto random=[&](){rng=rng*0x015a4e35u+1;return (rng>>16)&0x7fff;};bool radial=kind==SarielPatternKind::RadialStacksAndLasers;unsigned duration=radial ? 450:301,angle=0,total=0,attempts=0,spawned=0,skipped=0;std::vector<unsigned> sounds;
  for(unsigned tick=1;tick<=duration;++tick){
   Pellets.native_reset_all();std::vector<std::pair<int,int>> volley;int rain_x=0,rain_angle=64;bool laser_tick=radial && tick>200 && tick<=400 && tick%15==0;unsigned slot=laser_tick ? ((int(tick)-215)/15)%10:0;auto old=shootout_lasers[slot];
   if(radial){if(tick>=150 && tick<200 && tick%10==0)sounds.push_back(6);if(tick>200 && tick<=400 && tick%5==0){sounds.push_back(7);if(!laser_tick)volley.push_back({angle,100});volley.push_back({int(angle)-4,96});volley.push_back({int(angle)+4,96});if(difficulty>=2){volley.push_back({int(angle)-4,64});volley.push_back({int(angle)+4,64});if(difficulty==3){volley.push_back({angle,64});volley.push_back({angle,48});}}}}
   else{if(tick==50 || tick==100 || tick==150 || tick==200)sounds.push_back(6);if(tick%8==0 || tick==301){rain_x=random()%640;if(difficulty==3)rain_angle=(random()&63)+32;volley.push_back({rain_angle,44});}}
   require(attack.advance()==(tick==duration),"Original combined/radial completion boundaries");require(native_random_seed==rng,"Only rain consumes RNG, including post-reset final call");require(Pellets.native_alive_count()==int(volley.size()),"Original exact stack sizes and rain cadence");
   for(unsigned i=0;i<volley.size();++i){auto& p=Pellets.native_at(i);pixel_t vx,vy;vector2(vx,vy,volley[i].second+difficulty*6,static_cast<unsigned char>(volley[i].first));require(p.cur_left.v==(radial ? 320:rain_x)*16 && p.cur_top.v==(radial ? 160:64)*16 && p.velocity.x.v==vx && p.velocity.y.v==vy,"Original radial angle/stack speed or top-rain origin and velocity");require(p.from_group==(radial ? PG_NONE:difficulty==3 ? PG_1_RANDOM_WIDE:PG_1),"Original radial singles and rank rain group");trace^=std::uint16_t(p.velocity.x.v);trace*=1099511628211ull;}total+=volley.size();
   if(laser_tick){++attempts;auto& l=shootout_lasers[slot];if(old.alive){++skipped;require(l.age==old.age && l.origin_left.v==old.origin_left.v && l.step_x.v==old.step_x.v && l.step_y.v==old.step_y.v,"Occupied original laser slot rejects respawn");}else{++spawned;int tx=polar_x(320,600,static_cast<unsigned char>(angle)),ty=polar_y(160,600,static_cast<unsigned char>(angle));auto a=iatan2(ty-160,tx-320);const int speeds[]={7,8,8,9};require(l.alive && l.age==0 && l.origin_left.v==320*256 && l.origin_y.v==160*256 && l.step_x.v==Cos8(a) && l.step_y.v==Sin8(a) && l.ray_extend_speed==speeds[difficulty] && l.width_cel==3 && l.col==7 && l.moveout_at_age==20,"Original radial cyclic-slot laser geometry/speed/width/lifetime");}}
   require(shots.sound_commands.size()==sounds.size(),"Original prelude and volley sound schedule");for(unsigned i=0;i<sounds.size();++i)require(shots.sound_commands[i]==sounds[i],"Original SE6/SE7 command sequence");if(radial && tick>200 && tick<=400 && tick%5==0)angle=(angle+12)&255;update_shootout_lasers();
   if(argc>2 && radial && difficulty==1 && seed==1 && tick==300)dump_bmp(argv[2],*fb);
  }
  if(radial)require(total==(difficulty<2 ? 107u:difficulty==2 ? 187u:267u) && attempts==13 && spawned==12 && skipped==1 && sounds.size()==45,"Whole radial attack totals and original duplicate busy slot");else require(total==38 && sounds.size()==4,"Combined rain includes final frame-reset volley");require(!player_is_hit,"Invincibility respected throughout sustained lasers");auto before=fb->pages;require(attack.advance() && attack.frame()==0 && before==fb->pages,"Finished attacks remain inert");for(unsigned i=0;i<400;++i)update_shootout_lasers();for(auto& l:shootout_lasers)require(!l.alive,"All lasers drain after pattern");trace^=rng;trace*=1099511628211ull;
 }
 std::cout<<"Sariel laser/rain and radial stacks: four ranks, final reset volley, cyclic busy slots, RNG/sounds and drain passed; trace "<<std::hex<<trace<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
