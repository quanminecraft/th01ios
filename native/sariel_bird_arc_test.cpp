#include "sariel_birds.hpp"
#include "boss_entity_backend.hpp"
#include "pellet_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
struct Model {double x,y,vx,vy;int hatch;bool alive;};
int main(int argc,char** argv){try{
 if(argc<2)throw std::invalid_argument("Usage: sariel_bird_arc_test ASSET_DIR [BMP]");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS6_A1.GRP"));auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);BossEntityBackend banks{*fb,archive,{}};BossEntityBinding eb(banks);std::uint64_t trace=1469598103934665603ull;unsigned choices=0;
 for(unsigned difficulty=0;difficulty<4;++difficulty)for(unsigned seed=1;seed<=8;++seed){
  for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);fb->palette=bg.palette;page_access(0);player_invincible=true;player_is_hit=false;player_left=304;rank=difficulty;native_pellet_speed=0;shots.sound_commands.clear();SarielBody body;body.capture_wand_background();SarielBirds birds(archive);birds.begin_arc(body,seed);std::uint32_t rng=seed;auto random=[&](){rng=rng*0x015a4e35u+1;return (rng>>16)&0x7fff;};int sx=340*16,sy=64*16,vx=0,vy=0;unsigned group=0;std::vector<std::pair<int,int>> eggs;std::vector<Model> models;const int intervals[]={20,15,10,8};unsigned total=0;
  for(unsigned tick=1;tick<=400;++tick){
   Pellets.native_reset_all();for(auto& m:models){if(!m.alive)continue;if(m.hatch>0)--m.hatch;if(m.hatch==0){m.x+=m.vx;m.y+=m.vy;if(m.x<0 || m.x>639 || m.y<0 || m.y>399)m.alive=false;}}
   if(tick==50){vx=(4-(random()%2)*8)*16;vy=(2-(random()%2)*4)*16;}
   else if(tick>50 && tick<200){vx+=sx<320*16 ? 1:-1;vy+=sy<164*16 ? 1:-1;sx+=vx;sy+=vy;if(tick%intervals[difficulty]==0 && sx>=0 && sx<=608*16 && sy>=0 && sy<=368*16)eggs.push_back({sx>>4,sy>>4});}
   else if(tick==200){for(auto i=eggs.rbegin();i!=eggs.rend();++i){pixel_t x,y;vector2(x,y,3,(random()&127)+128);models.push_back({double(i->first),double(i->second),double(x),double(y),25,true});}group=random()%2;choices|=1u<<group;}
   require(birds.advance_arc()==(tick==400),"Original bird arc ends at 400");require(native_random_seed==rng,"Spawner directions, reverse egg velocities and group consume original RNG order");
   for(unsigned i=0;i<30;++i){auto s=birds.state(i);if(i>=models.size()){require(!s.alive,"No extra bird slots spawned");continue;}auto m=models[i];require(s.alive==m.alive && s.hatch==m.hatch && s.left==m.x && s.top==m.y && s.vx==m.vx && s.vy==m.vy,"Bird-before-pattern update, reverse egg order and exact hatch/movement");}
   unsigned count=0;if(tick>=200 && tick<400 && (tick%16==0 || tick%16==2))for(auto m:models)if(m.alive && m.hatch==0 && (group || (int(m.y+12)>=56 && int(m.y+12)<=400 && int(m.x+12)<640)))count+=group ? 6:1;count=std::min(count,100u);if(Pellets.native_alive_count()!=int(count))throw std::runtime_error("Arc count rank="+std::to_string(difficulty)+" seed="+std::to_string(seed)+" tick="+std::to_string(tick)+" group="+std::to_string(group)+" actual="+std::to_string(Pellets.native_alive_count())+" expected="+std::to_string(count));
   unsigned index=0;if(count)for(auto m:models){if(!m.alive || m.hatch || (!group && (int(m.y+12)<56 || int(m.y+12)>400 || int(m.x+12)>=640)))continue;for(unsigned j=0;j<(group ? 6u:1u) && index<count;++j){auto& p=Pellets.native_at(index++);require(p.cur_left.v==int(m.x+12)*16 && p.cur_top.v==int(m.y+12)*16,"Arc firing uses post-movement bird muzzle");require(p.from_group==(group ? PG_NONE:PG_1_AIMED),"Arc excludes gravity rain group");trace^=std::uint16_t(p.cur_left.v);trace*=1099511628211ull;trace^=std::uint16_t(p.velocity.x.v);trace*=1099511628211ull;}}total+=count;
   if(argc>2 && difficulty==1 && seed==1 && tick==240)dump_bmp(argv[2],*fb);
  }
  require(!eggs.empty() && total>0 && !player_is_hit,"Arc produces eggs and mature bird volleys");require(shots.sound_commands.size()==1 && shots.sound_commands[0]==8,"Original summon SE8 only");auto before=fb->pages;require(birds.advance_arc() && birds.arc_frame()==0 && before==fb->pages,"Completed arc calls are inert");
 }
 require(choices==3,"Seeds cover both aimed and six-ring arc groups");std::cout<<"Original Sariel bird arc: four ranks, egg trajectory, reverse hatch, RNG and firing order passed; trace "<<std::hex<<trace<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
