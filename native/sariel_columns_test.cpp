#include "sariel_columns.hpp"
#include "pellet_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
struct Expected{int x,y,a,speed;};
int main(int argc,char** argv){try{
 if(argc<2)throw std::invalid_argument("Usage: sariel_columns_test ASSET_DIR [BMP]");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS6_A4.GRP"));auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);std::uint64_t trace=1469598103934665603ull;unsigned suppressed=0,early_rain=0,outside=0;
 for(unsigned difficulty=0;difficulty<4;++difficulty)for(int target:{0,304,608})for(unsigned seed:{1u,123u}){
  for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);fb->palette=bg.palette;page_access(0);rank=difficulty;native_pellet_speed=0;player_left=target;shots.sound_commands.clear();SarielColumns attack(archive,seed);std::uint32_t rng=seed;int left=2,y=399;const int intervals[]={16,13,10,8};unsigned sounds=0,total=0;
  auto random=[&](){rng=rng*0x015a4e35u+1;return (rng>>16)&0x7fff;};
  for(unsigned tick=1;tick<=601;++tick){
   Pellets.native_reset_all();std::vector<Expected> expected;
   if(tick==50){left=2;y=399;}else if(tick>50 && tick<130)left+=8;else if(tick==130)left=0;
   else if(tick>150 && tick<350){if(tick%10==0){for(int i=0;i<4;++i)for(unsigned side=0;side<2;++side){int x=side ? 639-left:left;if(std::abs(target+16-x)>32)expected.push_back({x,y-8,192,48+i*16});else ++suppressed;}left+=32;++sounds;}}
   else if(tick==350){left=0;y=64;}
   else if(tick>130 && tick<550 && tick%intervals[difficulty]==0){for(unsigned i=0;i<2;++i)for(unsigned side=0;side<2;++side){int a=iatan2(400-y,int(random()%640)-left);int x=side ? 639-left:left;expected.push_back({x,y,a,48});outside+=x<0 || x>=640;}left+=32;if(tick<=150)++early_rain;}
   require(attack.advance()==(tick==601),"Original column/rain attack ends at 601");require(native_random_seed==rng,"Original four random targets per rain volley in left/right order");require(Pellets.native_alive_count()==int(expected.size()),"Exact column suppression and early/late rain cadence");
   for(unsigned i=0;i<expected.size();++i){auto e=expected[i];auto& p=Pellets.native_at(i);pixel_t vx,vy;vector2(vx,vy,e.speed+difficulty*6,static_cast<unsigned char>(e.a));require(p.cur_left.v==e.x*16 && p.cur_top.v==e.y*16 && p.velocity.x.v==vx && p.velocity.y.v==vy && p.from_group==PG_NONE,"Original column stack speeds and asymmetric rain aiming");trace^=std::uint16_t(p.cur_left.v);trace*=1099511628211ull;trace^=std::uint16_t(p.velocity.x.v);trace*=1099511628211ull;}total+=expected.size();require(shots.sound_commands.size()==sounds,"Original column firing sound cadence");
   if(argc>2 && difficulty==1 && target==304 && seed==1 && tick==90)dump_bmp(argv[2],*fb);
  }
  require(sounds==19 && total>100,"Nineteen bottom stack opportunities plus rank-dependent rain");for(auto s:shots.sound_commands)require(s==7,"Original column SE7");auto before=fb->pages;require(attack.advance() && attack.frame()==0 && before==fb->pages,"Completed attack remains inert");
 }
 require(suppressed>0 && early_rain>0 && outside>0,"Coverage includes player-safe columns, early rain branch and offscreen spawners");std::cout<<"Original Sariel columns/rain: four ranks, safe columns, early rain, ordered RNG/offscreen origins and completion passed; trace "<<std::hex<<trace<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
