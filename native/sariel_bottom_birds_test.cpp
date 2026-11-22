#include "sariel_birds.hpp"
#include "pellet_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
struct Model{double x,y,vx,vy;int hatch;bool alive;};
int main(int argc,char** argv){try{
 if(argc<2)throw std::invalid_argument("Usage: sariel_bottom_birds_test ASSET_DIR [BMP]");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS6_A4.GRP"));auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);std::uint64_t trace=1469598103934665603ull;
 for(unsigned difficulty=0;difficulty<4;++difficulty)for(unsigned seed:{1u,3u,123u}){
  for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);fb->palette=bg.palette;page_access(0);player_invincible=true;player_is_hit=false;rank=difficulty;native_pellet_speed=0;shots.sound_commands.clear();SarielBirds birds(archive);birds.begin_bottom(seed);std::uint32_t rng=seed;std::vector<Model> model;unsigned total=0;
  for(unsigned tick=1;tick<=300;++tick){
   Pellets.native_reset_all();for(auto& m:model){if(!m.alive)continue;if(m.hatch>0)--m.hatch;if(m.hatch==0){m.x+=m.vx;m.y+=m.vy;if(m.x<0 || m.x>639 || m.y<0 || m.y>399)m.alive=false;}}
   if(tick>=60 && tick<=90 && tick%10==0){int left=2+int(tick-50)*2;pixel_t vx,vy;auto angle=iatan2(-367,(tick-60)*10);vector2(vx,vy,4,angle);model.push_back({double(left),367,double(vx),double(vy),25,true});model.push_back({double(639-left),367,double(-vx),double(vy),25,true});}
   std::vector<unsigned> angles;bool fires=(tick>50 && tick<100 && tick%6==0) || (tick>100 && tick%10==0);if(fires)for(auto m:model)if(m.alive && m.hatch==0){rng=rng*0x015a4e35u+1;angles.push_back(((rng>>16)&127)+128);}
   require(birds.advance_bottom()==(tick==300),"Bottom bird attack finishes after final frame-300 firing call");require(native_random_seed==rng,"One RNG angle per mature live bird in slot order");
   for(unsigned i=0;i<30;++i){auto s=birds.state(i);if(i>=model.size()){require(!s.alive,"Only eight bottom birds spawn");continue;}auto m=model[i];require(s.alive==m.alive && s.hatch==m.hatch && s.left==m.x && s.top==m.y && s.vx==m.vx && s.vy==m.vy,"Original mirrored positions and velocities with bird-before-pattern hatch timing");}
   require(Pellets.native_alive_count()==int(angles.size()),"Original every-six/every-ten firing cadence and maturity gate");unsigned i=0;for(auto m:model){if(!fires || !m.alive || m.hatch)continue;auto& p=Pellets.native_at(i);pixel_t vx,vy;vector2(vx,vy,64+difficulty*6,angles[i]);require(p.cur_left.v==int(m.x+12)*16 && p.cur_top.v==int(m.y+12)*16 && p.velocity.x.v==vx && p.velocity.y.v==vy && p.motion_type==PM_GRAVITY && p.speed.v==1,"Original rain muzzle, rank speed, random angle and gravity acceleration");trace^=std::uint16_t(p.velocity.x.v);trace*=1099511628211ull;trace^=std::uint16_t(p.cur_left.v);trace*=1099511628211ull;++i;}total+=angles.size();
   if(argc>2 && difficulty==1 && seed==1 && tick==90)dump_bmp(argv[2],*fb);
  }
  require(model.size()==8 && total>20 && !player_is_hit,"Eight symmetric birds produce sustained rain volleys");require(shots.sound_commands.size()==4,"Four original summon sounds");for(auto s:shots.sound_commands)require(s==6,"Summon SE6");auto before=fb->pages;require(birds.advance_bottom() && birds.bottom_frame()==0 && before==fb->pages,"Completed bottom pattern is inert");
 }
 std::cout<<"Original Sariel bottom birds: mirrored spawns, debris/rays, hatch order, four-rank rain and completion passed; trace "<<std::hex<<trace<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
