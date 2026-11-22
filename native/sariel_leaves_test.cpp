#include "sariel_leaves.hpp"
#include "pellet_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
struct Model{int flag=0,x=0,y=0,vx=0,vy=0;};
int main(int argc,char** argv){try{
 if(argc<2)throw std::invalid_argument("Usage: sariel_leaves_test ASSET_DIR [BMP]");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS6_A6.GRP"));auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);std::uint64_t trace=1469598103934665603ull;unsigned seen=0,contacts=0,reversals=0,full=0,freed=0;
 for(unsigned seed:{1u,123u})for(unsigned interval:{1u,20u,32u,500u})for(bool invincible:{false,true}){
  for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);fb->palette=bg.palette;page_access(0);SarielLeaves leaves(archive,seed);std::array<Model,30> model{};std::uint32_t rng=seed;player_invincible=invincible;auto random=[&](){rng=rng*0x015a4e35u+1;return (rng>>16)&0x7fff;};
  for(unsigned frame=1;frame<=1800;++frame){
   if(frame%interval==0){bool spawned=false;for(auto& m:model)if(!m.flag){m.x=random()%6400;m.y=1200+random()%1000;int target=random()%640;pixel_t vx,vy;vector2(vx,vy,40,iatan2(64-m.y/10,target-m.x/10));m.vx=vx;m.vy=vy;m.flag=1;spawned=true;break;}full+=!spawned;}
   if(frame%2==0){for(auto& m:model){if(!m.flag)continue;if(m.flag==1){m.x+=m.vx;m.y+=m.vy;if(m.y<640)m.flag=2;}else if(m.flag<=5){if(m.flag==5){m.flag=6;m.vy=3+random()%2;m.vx=2;}}else if(m.flag==6){m.x+=m.vx;m.y+=m.vy;--m.vy;if(m.vy< -1){m.vy=30+random()%20;m.vx=m.vx<0 ? 20:-20;++reversals;}if(m.y/10>400){m.flag=0;++freed;}}}for(auto& m:model)if(m.flag>=2 && m.flag<=5 && frame%4==0)++m.flag;}
   player_left=304;for(auto m:model)if(m.flag==6 && m.y>3680 && m.y<3920 && m.x/10>0 && m.x/10<632){player_left=std::min(608,std::max(0,m.x/10-1));break;}bool expected_hit=false;if(frame%2==0 && !invincible)for(auto m:model)if(m.flag==6 && m.y>3680 && m.y<3920 && m.x/10>player_left && m.x/10<player_left+24)expected_hit=true;
   player_is_hit=false;leaves.update(frame,interval);require(native_random_seed==rng,"Original leaf RNG order: first-free spawn, splash exit and sway reset");require(player_is_hit==expected_hit,"Original even-tick leaf hitbox and invincibility");contacts+=player_is_hit;
   for(unsigned i=0;i<30;++i){auto s=leaves.state(i);auto m=model[i];require(s.flag==m.flag,"Original leaf flag lifecycle");if(m.flag){require(s.left==m.x && s.top==m.y && s.vx==m.vx && s.vy==m.vy,"Exact decimal-subpixel leaf position/velocity");seen|=1u<<m.flag;trace^=std::uint16_t(s.left);trace*=1099511628211ull;}}
   if(frame==20 && interval==20){auto m=model[0];require(m.flag==1 && m.x>=0 && m.x<6320 && m.y>=0 && m.y<3920,"First spark is on screen for raster check");const unsigned char dots[]={16,24,24,127,254,24,24,8};for(int y=0;y<8;++y)for(int x=0;x<8;++x){int px=m.x/10+x,py=m.y/10+y;require(fb->pixel(0,px,py)==((dots[y]&(128>>x)) ? 7:fb->pixel(1,px,py)),"Original unaligned 8x8 spark mask");}}
   if(argc>2 && seed==1 && interval==20 && invincible && frame==700)dump_bmp(argv[2],*fb);
  }
  auto before=fb->pages;auto previous_rng=native_random_seed;leaves.reset();for(unsigned i=0;i<30;++i)require(leaves.state(i).flag==0,"999 command clears every slot");require(fb->pages==before && native_random_seed==previous_rng,"Reset neither draws nor consumes RNG");
 }
 require((seen&(1u<<1)) && (seen&(1u<<2)) && (seen&(1u<<6)) && contacts>0 && reversals>0 && full>0 && freed>0,"Coverage includes all leaf stages, hits, sway reversal, capacity and retirement");std::cout<<"Original Sariel leaves: decimal movement, lifecycle/capacity/reset, RNG, spark raster and contact passed; trace "<<std::hex<<trace<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
