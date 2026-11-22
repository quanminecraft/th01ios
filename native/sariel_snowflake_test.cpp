#include "sariel_patterns.hpp"
#include "sariel_body.hpp"
#include "boss_entity_backend.hpp"
#include "pellet_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 if(argc<2)throw std::invalid_argument("Usage: sariel_snowflake_test ASSET_DIR [BMP]");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS6_A2.GRP"));auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);BossEntityBackend banks{*fb,archive,{}};BossEntityBinding eb(banks);std::uint64_t trace=1469598103934665603ull;
 for(unsigned difficulty=0;difficulty<4;++difficulty)for(int target:{40,304,560})for(bool invincible:{false,true}){
  for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);fb->palette=bg.palette;page_access(0);rank=difficulty;native_pellet_speed=0;player_left=target;player_invincible=invincible;shots.sound_commands.clear();SarielBody body;body.capture_wand_background();SarielPattern attack(123,SarielPatternKind::SnowflakeWithRings,&body);int phase=-1,wand_frame=0,left=0,top=0,vx=0,vy=0,radius=0;unsigned ring_angle=0,launches=0,detonations=0,changes=0,hits=0;std::uint32_t rng=123;std::vector<unsigned> sounds;const int intervals[]={18,12,10,8};auto random=[&](){rng=rng*0x015a4e35u+1;return (rng>>16)&0x7fff;};
  for(unsigned tick=1;tick<=480;++tick){
   Pellets.native_reset_all();player_is_hit=false;bool expected_hit=false,early=false;
   if(tick<10)phase=-1;if(tick%160==0)phase=0;
   if(phase==0){if(++wand_frame==16){wand_frame=0;phase=1;}}
   if(phase==1){++launches;sounds.push_back(6);left=340*16;top=64*16;pixel_t x,y;vector2(x,y,112,iatan2(316,target+12-340));vx=x;vy=y;phase=2;}
   if(phase==2){top+=vy;if(top>=396*16){phase=3;early=true;}else left+=vx;}
   if(!early){
    if(phase==3){++detonations;radius=48;(void)random();sounds.push_back(10);}
    if(phase>=3)++phase;
    if(phase>=5 && phase%8==0){radius-=6;(void)random();(void)random();++changes;if(radius<=8){phase=-1;early=true;}}
    if(!early && !invincible && (left/16+32)>target && (left/16-64)<target && phase>=3)expected_hit=true;
   }
   require(attack.advance()==(tick==480),"Combined snowflake and rings ends at 480");require(native_random_seed==rng,"Original detonation and star RNG timing");require(player_is_hit==expected_hit,"Original detonation horizontal hitbox and invincibility");require(shots.sound_commands.size()==sounds.size(),"Original launch and detonation sound timing");for(unsigned i=0;i<sounds.size();++i)require(shots.sound_commands[i]==sounds[i],"Original SE6/SE10 sequence");
   unsigned count=tick%intervals[difficulty]==0 ? 4:0;require(Pellets.native_alive_count()==int(count),"Orb rings continue throughout snowflake movement and explosion");for(unsigned i=0;i<count;++i){auto& p=Pellets.native_at(i);int a=i==0 ? -int(ring_angle):i==1 ? 128-int(ring_angle):i==2 ? ring_angle:128+ring_angle;pixel_t x,y;vector2(x,y,56+difficulty*6,static_cast<unsigned char>(a));require(p.cloud_frame==1 && p.cur_left.v==(i<2 ? 568:64)*16 && p.cur_top.v==160*16 && p.velocity.x.v==x && p.velocity.y.v==y,"Original ring cloud, origin and mirrored velocity during combined attack");}if(count)ring_angle=(ring_angle+intervals[difficulty])&255;
   hits+=expected_hit;trace^=rng;trace*=1099511628211ull;trace^=player_is_hit;trace*=1099511628211ull;
   if(argc>2 && difficulty==1 && target==304 && invincible && phase==16 && detonations==1)dump_bmp(argv[2],*fb);
  }
  require(launches==2 && detonations==2 && changes==14,"Two complete snowflakes, each with seven shrinking explosion updates");require(invincible ? hits==0:hits>0,"Detonation contacts vulnerable aimed player");auto before=fb->pages;require(attack.advance() && attack.frame()==0 && before==fb->pages,"Completed combined pattern is inert");
 }
 std::cout<<"Original Sariel snowflake plus rings: four ranks, three aims, explosion RNG/sounds/hitbox and invincibility passed; trace "<<std::hex<<trace<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
