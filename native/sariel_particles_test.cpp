#include "sariel_particles.hpp"
#include "pellet_backend.hpp"
#include "assets.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
#include <cmath>
using namespace th01_native;
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
struct Model {bool alive=false;double x=0,y=0,v=0;int age=0;};
int main(int argc,char** argv){try{
 if(argc<2)throw std::invalid_argument("Usage: sariel_particles_test ASSET_DIR [BMP]");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto ref=std::make_unique<Framebuffer>();auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);std::uint64_t trace=1469598103934665603ull;unsigned retired=0,crossed=0;
 for(auto kind:{SarielParticleKind::Vertical,SarielParticleKind::Wavy,SarielParticleKind::Horizontal})for(bool bottom:{false,true})for(unsigned seed:{1u,123u}){
  for(int y=0;y<400;++y)for(int x=0;x<640;++x)for(unsigned p=0;p<2;++p)fb->put(p,x,y,x%16<4 ? 3:12);ref->pages=fb->pages;for(unsigned i=0;i<16;++i)fb->palette[i]=0xff000000u|i*0x111111u;page_access(0);SarielParticles particles(seed);std::array<Model,30> model{};std::uint32_t rng=seed;auto random=[&](){rng=rng*0x015a4e35u+1;return (rng>>16)&0x7fff;};
  auto draw=[&](const Model& m,unsigned color){int offset,first;if(kind==SarielParticleKind::Wavy){int wx=polar_y(pixel_t(m.x),pixel_t(16),m.age);offset=std::int16_t((wx>>3)+int(m.y)*80);first=wx&7;}else{offset=std::int16_t(m.x/8+m.y*80);first=std::int16_t(m.x)%8;if(kind==SarielParticleKind::Vertical && first==7)first=0;}
   bool split=kind==SarielParticleKind::Horizontal && first==7;crossed+=split;for(int y=0;y<2;++y)for(int byte=0;byte<(split ? 2:1);++byte){unsigned mask=byte ? 128:192>>first;for(int bit=0;bit<8;++bit)if(mask&(128>>bit)){int linear=(offset+y*80+byte)*8+bit;if(linear>=0 && linear<640*400 && ref->pixel(1,linear%640,linear/640)==12)ref->put(0,linear%640,linear/640,color);}}};
  for(unsigned frame=1;frame<=700;++frame){
   if(frame%7==0)for(auto& m:model)if(!m.alive){m.x=random()%640;if(kind==SarielParticleKind::Vertical){m.y=bottom ? 397:0;m.v=bottom ? -int(random()%15)-8:int(random()%15)+2;}else if(kind==SarielParticleKind::Wavy){m.y=400;m.v=-1;m.age=0;}else{m.y=random()%400;m.v=random()%2==0 ? -6:6;}m.alive=true;break;}
   if(frame%2==0)for(auto& m:model){if(!m.alive)continue;draw(m,12);if(kind==SarielParticleKind::Vertical){m.y+=m.v;auto vo=std::int16_t(m.x/8+m.y*80);if(vo>=399*80 || vo<0)m.alive=false;}else if(kind==SarielParticleKind::Wavy){m.y+=m.v;++m.age;if(m.age>=100)m.alive=false;}else{m.x+=m.v;m.v+=m.v<0 ? 0.1:-0.1;if(m.x>=640 || m.x<0 || std::abs(m.v)<0.5)m.alive=false;}if(m.alive)draw(m,4);else ++retired;}
   particles.update(kind,frame,bottom);require(native_random_seed==rng,"Original particle RNG spawn order");for(unsigned i=0;i<30;++i){auto s=particles.state(kind,i);auto m=model[i];require(s.alive==m.alive,"Original particle retirement timing");if(m.alive)require(s.left==m.x && s.top==m.y && s.velocity==m.v && s.age==m.age,"Original vertical/wavy/double-horizontal motion");}if(fb->pages!=ref->pages)throw std::runtime_error("Particle raster mismatch kind="+std::to_string(unsigned(kind))+" frame="+std::to_string(frame));trace^=rng;trace*=1099511628211ull;for(auto& plane:fb->pages[0]){trace^=plane[(frame*43)%32000];trace*=1099511628211ull;}
   if(argc>2 && kind==SarielParticleKind::Horizontal && !bottom && seed==1 && frame==500)dump_bmp(argv[2],*fb);
  }
  auto before=fb->pages;auto previous_rng=native_random_seed;particles.reset();for(auto k:{SarielParticleKind::Vertical,SarielParticleKind::Wavy,SarielParticleKind::Horizontal})for(unsigned i=0;i<30;++i)require(!particles.state(k,i).alive,"Host reload reset clears all particle banks");require(before==fb->pages && previous_rng==native_random_seed,"Particle reset neither draws nor consumes RNG");
 }
 require(retired>0 && crossed>0,"Coverage includes retirement and horizontal byte-boundary split");std::cout<<"Original Sariel particles: all three modes, both vertical directions, RNG/motion and per-frame compare-mask raster passed; trace "<<std::hex<<trace<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
