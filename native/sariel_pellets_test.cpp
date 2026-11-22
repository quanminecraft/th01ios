#include "sariel_patterns.hpp"
#include "pellet_backend.hpp"
#include "assets.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 if(argc<2)throw std::invalid_argument("Usage: sariel_pellets_test ASSET_DIR");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);std::uint64_t trace=1469598103934665603ull;
 for(unsigned difficulty=0;difficulty<4;++difficulty)for(auto kind:{SarielPatternKind::OrbRings,SarielPatternKind::SlingClusters,SarielPatternKind::Semicircles})for(unsigned seed:{1u,123u}){
  rank=difficulty;native_pellet_speed=0;Pellets.native_reset_all();SarielPattern attack(seed,kind);std::uint32_t rng=seed;unsigned total=0;unsigned angle=0;const int intervals[]={18,12,10,8},sling_intervals[]={4,3,2,2};unsigned duration=kind==SarielPatternKind::OrbRings ? 480:kind==SarielPatternKind::SlingClusters ? 299:151;
  for(unsigned tick=1;tick<=duration;++tick){
   Pellets.native_reset_all();unsigned count=0;int origin_x=320,origin_y=160,base=0,step=0,speed=0;
   if(kind==SarielPatternKind::OrbRings){if(tick%intervals[difficulty]==0){count=4;speed=56;}}
   else if(kind==SarielPatternKind::SlingClusters){if(tick%200>=150 && tick%sling_intervals[difficulty]==0){count=1;rng=rng*0x015a4e35u+1;unsigned a=(rng>>16)&255;origin_x=polar_y(320,40,a);origin_y=polar_x(180,40,a);}}
   else {if(tick==50){count=20;base=0;step=7;speed=32;}if(tick==60){count=10;base=4;step=12;speed=48;}if(tick==70){count=8;base=4;step=14;speed=64;}if(tick==80){count=10;base=14;step=10;speed=48;}}
   require(attack.advance()==(tick==duration),"Original pellet pattern completion boundary");require(native_random_seed==rng,"Only sling origin sampling consumes RNG");require(Pellets.native_alive_count()==int(count),"Exact per-frame original projectile count");
   for(unsigned i=0;i<count;++i){auto& p=Pellets.native_at(i);int x=origin_x,y=origin_y,a=base+i*step;
    if(kind==SarielPatternKind::OrbRings){x=i<2 ? 568:64;y=160;a=i==0 ? -int(angle):i==1 ? 128-int(angle):i==2 ? angle:128+angle;require(p.cloud_frame==1 && !p.moving,"Orb ring pellets start with original cloud delay");}
    else require(p.cloud_frame==0 && p.moving,"Other two patterns spawn without clouds");
    require(p.cur_left.v==x*16 && p.cur_top.v==y*16 && p.from_group==PG_NONE,"Original pellet origin and single-pellet type");
    if(kind==SarielPatternKind::SlingClusters){require(p.motion_type==PM_SLING_AIMED && p.speed.v==72,"Sling uses fixed 4.5px motion speed");}
    else {pixel_t vx,vy;vector2(vx,vy,speed+difficulty*6,static_cast<unsigned char>(a));require(p.motion_type==PM_REGULAR && p.velocity.x.v==vx && p.velocity.y.v==vy,"Original angular progression and rank-tuned velocity");}
    trace^=std::uint16_t(p.cur_left.v);trace*=1099511628211ull;trace^=std::uint16_t(p.velocity.x.v);trace*=1099511628211ull;
   }
   if(kind==SarielPatternKind::OrbRings && count)angle=(angle+intervals[difficulty])&255;total+=count;
  }
  unsigned expected=kind==SarielPatternKind::OrbRings ? (480/intervals[difficulty])*4:kind==SarielPatternKind::Semicircles ? 48:difficulty==0 ? 12:difficulty==1 ? 17:25;
  require(total==expected,"Whole-pattern original pellet total");require(attack.advance() && attack.frame()==0 && native_random_seed==rng,"Completed pattern remains inert");
 }
 std::cout<<"Sariel orb rings, sling clusters and semicircles: four ranks, two seeds, exact volleys/motion/completion passed; trace "<<std::hex<<trace<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
