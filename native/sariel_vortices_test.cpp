#include "sariel_vortices.hpp"
#include "boss_entity_backend.hpp"
#include "pellet_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
struct Expected {int x,y,angle;bool group;};
int main(int argc,char** argv){try{
 if(argc<2)throw std::invalid_argument("Usage: sariel_vortices_test ASSET_DIR [BMP]");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS6_A1.GRP"));auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);BossEntityBackend banks{*fb,archive,{}};BossEntityBinding eb(banks);auto sprites=read_sprites(asset(archive,"BOSS6GR1.GRC"));std::uint64_t trace=1469598103934665603ull;unsigned branches=0;
 for(unsigned difficulty=0;difficulty<4;++difficulty)for(unsigned seed=1;seed<=16;++seed){
  for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);fb->palette=bg.palette;page_access(0);rank=difficulty;native_pellet_speed=0;SarielBody body;body.capture_wand_background();SarielVortices attack(body,archive,seed);std::uint32_t rng=seed;int x[2]={0,608},y[2]={200,200};int first=0,second=0;const int split[]={140,145,150,155};unsigned total=0;
  auto random=[&](){rng=rng*0x015a4e35u+1;return (rng>>16)&0x7fff;};
  for(unsigned tick=1;tick<=320;++tick){
   Pellets.native_reset_all();std::vector<Expected> expected;
   if(tick==200)first=random()%2;if(tick==240){second=random()%2;branches|=1u<<(first*2+second);}
   if(tick>=50 && tick<320)for(unsigned i=0;i<2;++i){
    if(tick<100){x[i]=i*608;y[i]=200;}else if(tick<unsigned(split[difficulty])){x[i]+=i ? -5:5;y[i]-=2;}else if(tick>=200 && tick<240)x[i]+=2-first*4;else if(tick>=240)x[i]-=second!=first ? 2-second*4:4-second*8;
    if(tick%4==0){if(tick<100)expected.push_back({x[i]+12,y[i]+12,64,true});else if(tick<unsigned(split[difficulty])){expected.push_back({x[i]+12,y[i]+12,64,true});expected.push_back({x[i]+12,y[i]+12,iatan2(412-y[i],i*160-68),false});}else{int a=iatan2(388-y[i],x[1-i]-x[i]);expected.push_back({x[i]+12,y[i]+12,64,false});expected.push_back({x[i]+12,y[i]+12,a,false});expected.push_back({x[i]+12,y[i]+12,128-a,false});}}
   }
   require(attack.advance()==(tick==320),"Vortex pattern finishes at frame 320");require(native_random_seed==rng,"Only two direction choices consume RNG");require(Pellets.native_alive_count()==int(expected.size()),"Original per-frame vortex volley counts");
   for(unsigned i=0;i<expected.size();++i){auto e=expected[i];auto& p=Pellets.native_at(i);pixel_t vx,vy;vector2(vx,vy,112+difficulty*6,static_cast<unsigned char>(e.angle));require(p.cur_left.v==e.x*16 && p.cur_top.v==e.y*16 && p.velocity.x.v==vx && p.velocity.y.v==vy && p.from_group==(e.group ? PG_1:PG_NONE),"Sequential vortex positions, asymmetric spread aim and rank velocities");trace^=std::uint16_t(p.velocity.x.v);trace*=1099511628211ull;trace^=std::uint16_t(p.cur_left.v);trace*=1099511628211ull;}total+=expected.size();
   if(tick==60){auto& sprite=sprites.images[0];for(int i=0;i<2;++i)for(int py=0;py<32;++py)for(int px=0;px<32;++px){bool erased_by_right_restore=(i==0 && px<16 && py>0);auto expected=(!erased_by_right_restore && sprite.opaque[py*32+px]) ? 7:fb->pixel(1,i*608+px,200+py);require(fb->pixel(0,i*608+px,200+py)==expected,"Vortex GRC cel uses original white color and transparency");}}
   if(argc>2 && difficulty==1 && seed==1 && tick==180)dump_bmp(argv[2],*fb);
  }
  require(total>250 && attack.frame()==0,"Whole vortex sequence emits sustained volleys");auto before=fb->pages;require(attack.advance() && before==fb->pages,"Completed vortex calls are inert");
 }
 require(branches==15,"Seeds exercise all first/second direction combinations");std::cout<<"Original Sariel vortices: four ranks, all direction branches, sprite mask, ordered spread angles and completion passed; trace "<<std::hex<<trace<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
