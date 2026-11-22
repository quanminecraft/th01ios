#include "elis_teleport.hpp"
#include "boss_entity_backend.hpp"
#include "pellet_backend.hpp"
#include "grp.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
 try{
    if(argc<2)throw std::invalid_argument("Usage: elis_bat_attacks_test ASSET_DIR");
    auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS5.GRP"));fb->palette=bg.palette;
    for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);
    auto stg=read_sprites(asset(archive,"STG.PTN")),miko=read_sprites(asset(archive,"MIKO.PTN"));
    ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding pb(player);
    BossEntityBackend entities{*fb,archive,{}};BossEntityBinding eb(entities);page_access(0);
    ElisTeleport elis;elis.entrance(archive,[](unsigned){});elis.begin_transform(true);for(unsigned i=0;i<101;++i)elis.advance_transform();auto bat=elis.bat();
    native_pellet_speed=0;player_invincible=true;player_is_hit=false;player_deflecting=false;orb_cur_left=orb_prev_left=0;orb_cur_top=orb_prev_top=380;
    std::uint64_t trace=1469598103934665603ull;
    for(unsigned difficulty=0;difficulty<4;++difficulty)for(unsigned pattern=1;pattern<=4;++pattern){
        rank=difficulty;native_random_seed=0x123;std::uint32_t rng=native_random_seed;unsigned total=0;shots.sound_commands.clear();
        for(unsigned tick=1;tick<=400;++tick){
            Pellets.native_reset_all();player_left=tick<=200 ? 100:500;unsigned count=0;pellet_group_t group=PG_NONE;unsigned random=0;
            if((pattern==2 || pattern==3) && tick%4==0){rng=rng*0x015a4e35u+1;random=(rng>>16)&0x7fff;}
            if(pattern==1 && tick%16==0){const pellet_group_t groups[]={PG_3_SPREAD_WIDE_AIMED,PG_5_SPREAD_WIDE_AIMED,PG_5_SPREAD_NARROW_AIMED,PG_5_SPREAD_NARROW};group=groups[difficulty];count=difficulty==0 ? 3:5;}
            if(pattern==2 && tick%4==0){group=random&1 ? PG_2_SPREAD_NARROW_AIMED:PG_2_SPREAD_WIDE_AIMED;count=2;}
            if(pattern==3 && tick%4==0)count=1;
            if(pattern==4 && tick%4==0){if(tick%50<10){group=PG_2_SPREAD_NARROW_AIMED;count=2;}else if(tick%50>=25 && tick%50<35){group=PG_3_SPREAD_NARROW_AIMED;count=3;}}
            elis.fire_bat_pattern(pattern,tick);require(Pellets.native_alive_count()==int(count),"Original bat firing intervals and volley sizes");require(native_random_seed==rng,"Only random two-way/rain patterns consume one RNG draw per volley");
            for(unsigned i=0;i<count;++i){
                auto& p=Pellets.native_at(i);int x=bat.left+20,y=bat.top+12;require(p.cur_left.v==x*16 && p.cur_top.v==y*16,"Original bat center muzzle");
                require(p.from_group==group,"Original rank/random spread group");subpixel_t vx,vy,speed;int offset=0;
                if(pattern==3){speed=std::max(16,2+int(difficulty)*6);vector2(vx,vy,speed,static_cast<unsigned char>(random));require(p.motion_type==PM_GRAVITY && p.speed.v==1,"Rain retains 0.0625 gravity and rank/minimum-clamped launch speed");}
                else{
                    speed=(pattern==1 ? 56:pattern==2 ? 88:85)+difficulty*6;
                    if(count==2){int spread=group==PG_2_SPREAD_WIDE_AIMED ? 12:4;offset=i==0 ? spread:-spread;}
                    else if(count==3){const int offsets[]={0,9,-9};offset=pattern==1 ? offsets[i]:(i==0 ? 0:i==1 ? 4:-4);}
                    else{const int offsets[]={0,4,-4,8,-8};offset=offsets[i]*(difficulty==1 ? 2:1);}
                    if(difficulty==3 && pattern==1)vector2(vx,vy,speed,static_cast<unsigned char>(64+offset));
                    else{unsigned char aim=iatan2(player_center_y()-y,player_center_x()-4-x)+offset;vector2(vx,vy,speed,aim);}
                }
                require(p.velocity.x.v==vx && p.velocity.y.v==vy,"Original spread angles, aim origin and subpixel velocities");
                trace^=std::uint16_t(p.velocity.x.v);trace*=1099511628211ull;trace^=std::uint16_t(p.velocity.y.v);trace*=1099511628211ull;
            }
            total+=count;
        }
        unsigned expected=pattern==1 ? (difficulty==0 ? 75:125):pattern==2 ? 200:100;
        require(total==expected && shots.sound_commands.empty(),"Four hundred ticks emit original totals without extra sound commands");
    }
    Pellets.native_reset_all();native_random_seed=7;rank=RANK_NORMAL;elis.begin_flight(4);bool saw=false;unsigned ticks=0;
    while(true){bool done=elis.advance_flight();saw|=Pellets.native_alive_count()>0;Pellets.unput_update_render();if(done)break;if(++ticks>1000)throw std::runtime_error("Armed bat flight stalled");}
    require(saw && !player_is_hit,"Live flight emits and updates real pellets with invincible player");
    std::cout<<"Elis four original bat attacks across four ranks, velocities, RNG and live flight passed; trace "<<std::hex<<trace<<'\n';
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}