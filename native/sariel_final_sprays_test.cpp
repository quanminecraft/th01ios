#include "sariel_patterns.hpp"
#include "sariel_columns.hpp"
#include "pellet_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 if(argc<2)throw std::invalid_argument("Usage: sariel_final_sprays_test ASSET_DIR [BMP]");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS6_A6.GRP"));auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);std::uint64_t trace=1469598103934665603ull;unsigned clamped=0;
 for(unsigned difficulty=0;difficulty<4;++difficulty)for(unsigned seed:{1u,123u}){
  rank=difficulty;native_pellet_speed=0;const int steps[]={32,24,18,16};int step=steps[difficulty];
  for(bool twice:{false,true}){
   for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);fb->palette=bg.palette;page_access(0);shots.sound_commands.clear();SarielPattern attack(seed,twice ? SarielPatternKind::CurvedSprayTwice:SarielPatternKind::CurvedSprayOnce);int right=(640+step-1)/step,left=640/step+1;unsigned total=(right+left)*(twice ? 2:1),begin=twice ? 80:100,duration=begin+total-1;int target=0,speed=112,sub=0;unsigned emitted=0;
   for(unsigned tick=1;tick<=duration;++tick){
    Pellets.native_reset_all();require(attack.advance()==(tick==duration),"Original single/double sweep completion frame");require(native_random_seed==seed && shots.sound_commands.empty(),"Curved sprays consume no RNG or sound commands");require(Pellets.native_alive_count()==int(tick>=begin),"One pellet per firing tick, none during prelude");
    if(tick>=begin){auto& p=Pellets.native_at(0);pixel_t vx,vy;int tuned=std::max(16,speed+int(difficulty)*6);clamped+=speed+int(difficulty)*6<16;vector2(vx,vy,tuned,iatan2(215,target-320));require(p.cur_left.v==320*16 && p.cur_top.v==185*16 && p.velocity.x.v==vx && p.velocity.y.v==vy && p.motion_type==PM_REGULAR,"Original sweep aim and decreasing speed with minimum clamp");trace^=std::uint16_t(vx);trace*=1099511628211ull;trace^=std::uint16_t(vy);trace*=1099511628211ull;speed-=4;target+=(sub%2==0 ? step:-step);if(sub%2==0 && target>=640){++sub;target=640;speed=112;}else if(sub%2==1 && target<0){++sub;target=0;speed=112;}++emitted;}
    if(argc>2 && difficulty==1 && seed==1 && !twice && tick==101)dump_bmp(argv[2],*fb);
   }
   require(emitted==total && attack.frame()==0,"Original rank-dependent total sweep count");auto before=fb->pages;require(attack.advance() && before==fb->pages,"Finished spray remains inert");
  }
  {
   for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);shots.sound_commands.clear();SarielColumns attack(archive,seed,SarielColumnKind::SealRain);std::uint32_t rng=seed;unsigned count=30+difficulty*5;
   for(unsigned tick=1;tick<=104;++tick){Pellets.native_reset_all();require(attack.advance()==(tick==104),"Seal rain fires and ends at 104");require(Pellets.native_alive_count()==int(tick==104 ? count:0),"Seal rain waits for rising rays before its single volley");if(tick==104)for(unsigned i=0;i<count;++i){rng=rng*0x015a4e35u+1;auto& p=Pellets.native_at(i);pixel_t vx,vy;vector2(vx,vy,16+difficulty*6,static_cast<unsigned char>((rng>>16)&0x7fff));require(p.cur_left.v==int((640/count)*i)*16 && p.cur_top.v==185*16 && p.velocity.x.v==vx && p.velocity.y.v==vy && p.motion_type==PM_GRAVITY && p.speed.v==1,"Original evenly spaced seal rain, random full-circle angle and gravity");trace^=std::uint16_t(vx);trace*=1099511628211ull;}require(native_random_seed==rng,"Seal rain consumes exactly one angle per pellet at final frame");}
   require(shots.sound_commands.size()==7 && shots.sound_commands.back()==3,"Six ray sounds then SE3 seal burst");for(unsigned i=0;i<6;++i)require(shots.sound_commands[i]==6,"Rising ray SE6");auto before=fb->pages;require(attack.advance() && before==fb->pages,"Finished seal rain remains inert");
  }
 }
 require(clamped>0,"Higher rank sweeps exercise minimum pellet speed");std::cout<<"Original Sariel final sprays/seal rain: four ranks, sweep counts/speed clamps, RNG/sounds and completion passed; trace "<<std::hex<<trace<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
