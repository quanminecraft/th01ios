#include "sariel_birds.hpp"
#include "pellet_backend.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 if(argc<2)throw std::invalid_argument("Usage: sariel_birds_test ASSET_DIR");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto reference=std::make_unique<Framebuffer>();auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);auto sprites=read_sprites(asset(archive,"BOSS6GR2.GRC"));unsigned waits=0;std::uint64_t trace=1469598103934665603ull;
 for(unsigned run=0;run<2;++run){SarielBirds birds(archive,[&](unsigned ms){require(ms==100,"Original bird collision delay");++waits;});fb->clear(0,2);fb->clear(1,2);reference->clear(0,2);page_access(0);player_invincible=true;player_is_hit=false;birds.spawn(200,120,2,-0.5);
  for(unsigned frame=1;frame<=25;++frame){birds.update(frame);reference->blit(0,200,120,sprites.images[1+(frame-1)/5],true,15);if(frame==25)reference->blit(0,202,119,sprites.images[6+(frame/5)%2],true,15);auto s=birds.state(0);require(s.alive && s.hatch==25-int(frame) && s.left==(frame==25 ? 202:200) && s.top==(frame==25 ? 119.5:120),"25-frame hatch moves immediately on its final tick");require(fb->pages[0]==reference->pages[0],"Original accumulating hatch cels and first flight raster");}
  birds.update(26);auto s=birds.state(0);require(s.left==204 && s.top==119,"Original double precision movement continues after hatch");
  birds.reset();for(unsigned i=0;i<31;++i)birds.spawn(100+i,100,0,0);for(unsigned i=0;i<30;++i)require(birds.state(i).alive && birds.state(i).left==100+i,"Thirty-slot capacity and first-free spawn order");birds.reset();for(unsigned i=0;i<30;++i)require(!birds.state(i).alive,"Reset clears each alive flag");
  for(unsigned difficulty=0;difficulty<4;++difficulty)for(auto group:{SarielBirdFire::Aimed,SarielBirdFire::Ring,SarielBirdFire::Rain}){
   birds.reset();Pellets.native_reset_all();rank=difficulty;native_pellet_speed=0;native_random_seed=123;birds.spawn(200,120,0,0);birds.fire(group);require(Pellets.native_alive_count()==0,"Unhatched birds cannot fire");for(unsigned frame=1;frame<=25;++frame)birds.update(frame);birds.fire(group);unsigned count=group==SarielBirdFire::Ring ? 6:1;require(Pellets.native_alive_count()==int(count),"Original aimed/ring/rain volley size");std::uint32_t rng=123;unsigned rain_angle=0;if(group==SarielBirdFire::Rain){rng=rng*0x015a4e35u+1;rain_angle=((rng>>16)&127)+128;}require(native_random_seed==rng,"Only rain firing consumes one random angle");
   for(unsigned i=0;i<count;++i){auto& p=Pellets.native_at(i);require(p.cur_left.v==212*16 && p.cur_top.v==132*16,"Bird muzzle offsets are twelve pixels");if(group==SarielBirdFire::Aimed)require(p.from_group==PG_1_AIMED,"Aimed bird uses original aimed group");else{pixel_t vx,vy;vector2(vx,vy,64+difficulty*6,group==SarielBirdFire::Ring ? i*42:rain_angle);require(p.velocity.x.v==vx && p.velocity.y.v==vy && p.motion_type==(group==SarielBirdFire::Ring ? PM_REGULAR:PM_GRAVITY),"Original six-ring angles or random gravity rain");if(group==SarielBirdFire::Rain)require(p.speed.v==1,"Rain acceleration is one subpixel");}trace^=std::uint16_t(p.velocity.x.v);trace*=1099511628211ull;}
  }
  birds.reset();birds.spawn(639,399,1,0);for(unsigned frame=1;frame<=25;++frame)birds.update(frame);require(!birds.state(0).alive,"Bird leaves when its top-left passes the inclusive screen boundary");
  birds.reset();birds.spawn(304,368,0,0);player_left=304;for(unsigned frame=1;frame<=25;++frame)birds.update(frame);require(!player_is_hit,"Invincible player is immune to bird contact");player_invincible=false;birds.update(26);require(player_is_hit && waits==run+1,"Original contact hit and 100ms host delay");player_invincible=true;player_is_hit=false;
 }
 std::cout<<"Original Sariel birds: hatch raster, movement, capacity/reload, four-rank firing, clipping and contact passed; trace "<<std::hex<<trace<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
