#include "sariel_background.hpp"
#include "shot_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 if(argc<2)throw std::invalid_argument("Usage: sariel_background_test ASSET_DIR [BMP]");auto fb=std::make_unique<Framebuffer>();auto reference=std::make_unique<Framebuffer>();SpriteSet sprites;ShotBackend shots{*fb,sprites,{}};ShotBackendBinding sb(shots);std::uint64_t trace=1469598103934665603ull;
 for(unsigned target=1;target<=3;++target)for(unsigned seed:{1u,123u}){
  auto next=read_grp(read_file(std::filesystem::path(argv[1])/("BOSS6_A"+std::to_string(target+1)+".GRP")));for(int y=0;y<400;++y)for(int x=0;x<640;++x){auto color=(x/8+y/4)%4==0 ? 3:12;fb->put(0,x,y,color);fb->put(1,x,y,color);}reference->pages=fb->pages;fb->display_scroll_y=123;page_access(0);native_random_seed=seed;shots.sound_commands.clear();std::vector<unsigned> callbacks;Palette saved{};SarielBackgroundHooks hooks{[&](){callbacks.push_back(1);},[&](){callbacks.push_back(3);},[&](int hp){require(hp==11,"Original HP value forwarded to HUD");callbacks.push_back(4);},[&](const Palette& p){saved=p;callbacks.push_back(2);}};SarielBackground transition(argv[1],target,11,std::move(hooks));std::uint32_t rng=seed;auto random=[&](){rng=rng*0x015a4e35u+1;return (rng>>16)&0x7fff;};unsigned gust=0,xcell=0;
  for(unsigned tick=1;tick<=109;++tick){
   if(tick>=50){for(int row=0;row<100;++row){int base=row*4*80+xcell/8+random()%8;unsigned masks[10]{};for(int i=0;i<10;++i){int off=base-i*2;if(off/80!=base/80)break;if(off>=0 && off/80==row*4)for(int bit=0;bit<8;++bit)if(reference->pixel(1,(off%80)*8+bit,off/80)==12)masks[i]|=128>>bit;}
    for(int i=0;i<5;++i){int off=base-i*2;if(off/80!=base/80)break;if(off>=0 && off/80==row*4)for(int bit=0;bit<8;++bit)if(masks[i]&(128>>bit))reference->put(0,(off%80)*8+bit,off/80,2+(i>2 ? 4-i:i));}
    for(unsigned i=0;i<gust+2;++i){int off=base-10-int(i)*2;if(off/80!=base/80)break;if(off>=0 && off/80==row*4)for(int bit=0;bit<8;++bit)if(masks[5+i]&(128>>bit))reference->put(0,(off%80)*8+bit,off/80,12);}
   }xcell+=32;if(xcell==640){xcell=0;++gust;if(gust==3){for(unsigned p=0;p<2;++p)reference->blit(p,0,0,next.images[0],false);}}}
   require(transition.advance()==(tick==109),"Three original gusts complete after 109 host ticks");require(native_random_seed==rng,"Exactly one random stripe offset per cell row");if(fb->pages!=reference->pages)throw std::runtime_error("Stripe model mismatch tick="+std::to_string(tick)+" target="+std::to_string(target)+" seed="+std::to_string(seed));if(tick<109)require(callbacks.empty(),"HUD/palette callbacks wait until transition completes");for(auto& plane:fb->pages[0])for(auto byte:plane){trace^=byte;trace*=1099511628211ull;}
  }
  require(callbacks==std::vector<unsigned>({1,2,3,4}) && saved==next.palette && fb->palette==next.palette && fb->display_scroll_y==0 && current_render_page()==0,"Original final VSync, background/palette, HUD/HP order and page/scroll");require(shots.sound_commands==std::vector<unsigned>({10,10,10}),"Original SE10 at each gust start");auto before=fb->pages;require(transition.advance() && transition.frame()==0 && before==fb->pages,"Completed transition remains inert");if(argc>2 && target==2 && seed==1)dump_bmp(argv[2],*fb);
 }
 std::cout<<"Original Sariel background transitions: every stripe mask, three gusts, RNG, palette/HUD and completion passed; trace "<<std::hex<<trace<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
