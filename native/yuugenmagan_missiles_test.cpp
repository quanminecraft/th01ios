#include "yuugenmagan_eyes.hpp"
#include "boss_entity_backend.hpp"
#include "missile_backend.hpp"
#include "th01/math/vector.hpp"
#include "th01/rank.h"
#include "grp.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
    try{
        if(argc!=2)throw std::invalid_argument("Usage: yuugenmagan_missiles_test ASSET_DIR");
        auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);BossEntityBackend entities{*fb,archive,{}};BossEntityBinding eb(entities);MissileBackend missiles{archive};MissileBinding mb(missiles);
        std::uint64_t trace=1469598103934665603ull;bool variations[2][3]={};
        for(unsigned difficulty=0;difficulty<4;++difficulty)for(bool clock:{false,true})for(unsigned initial_seed:{1u,2u,3u,4u}){
            rank=difficulty;native_random_seed=initial_seed;player_left=300;player_invincible=true;player_is_hit=false;shots.sound_commands.clear();fb->clear(0,3);fb->clear(1,5);page_access(0);
            YuugenmaganEyes eyes;eyes.begin_missiles(clock);require(missiles.sprites.images.size()>=5,"Loads real moving/explosion missile sprites");unsigned ticks=0,pairs=0;std::uint32_t seed=initial_seed;int variation=-1;unsigned char initial_w=0,initial_e=0;
            while(eyes.missile_state().iterations<1){
                ++ticks;player_left=ticks<=100 ? 300 : 500;eyes.advance_missiles();auto s=eyes.missile_state();
                if(ticks==90)require(eyes.eye(2).cel==1 && eyes.eye(3).cel==1,"Preparation closes both southern eyes");
                if(ticks==100){seed=seed*0x015a4e35u+1;variation=((seed>>16)&0x7fff)%(clock ? 2 : 3);variations[clock][variation]=true;require(s.subphase==variation*4+1 && s.target==292,"One RNG choice and target snapshot at frame100");initial_w=static_cast<unsigned char>(iatan2(252,128)-16);initial_e=static_cast<unsigned char>(iatan2(252,-64)+16);require(s.angle_west==initial_w && s.angle_east==initial_e,"Original offset angle calculation");}
                bool firing=(ticks>=104 && ticks<=200 && ticks%8==0) || (ticks>=201 && ticks%8==1);require(Missiles.native_alive_count()==(firing ? 2u : 0u),"Original paired missile cadence including eleven shifted pairs per segment");
                if(ticks>100)require(s.target==292,"Missile aim stays locked after player moves");
                if(firing){
                    unsigned char expected_w=initial_w,expected_e=initial_e;
                    unsigned first=ticks<201 ? 0 : std::min(11u,(ticks-201)/8+1);unsigned second=ticks<289 ? 0 : (ticks-289)/8+1;
                    if(!clock){expected_w-=first*2;expected_e+=first*2;if(variation==0)expected_w+=second*4;else if(variation==1)expected_e-=second*4;else{expected_w+=second*2;expected_e-=second*2;}}
                    else{int delta=variation==0 ? -2 : 2;expected_w+=int(first)*delta;expected_e+=int(first)*delta;delta=variation==0 ? 3 : -3;expected_w+=int(second)*delta;expected_e+=int(second)*delta;}
                    require(s.angle_west==expected_w && s.angle_east==expected_e,"Original away/towards and clockwise variation angle shifts");
                    for(unsigned i=0;i<2;++i){auto m=Missiles.native_at(i);pixel_t vx,vy;vector2(vx,vy,pixel_t(8),static_cast<unsigned char>(i ? expected_e : expected_w));require(m.left==(i ? 412 : 220)*16 && m.top==204*16 && m.velocity_x==vx*16 && m.velocity_y==vy*16,"Original southern muzzle coordinates and integer eight-pixel velocities");trace^=std::uint16_t(m.velocity_x);trace*=1099511628211ull;trace^=std::uint16_t(m.velocity_y);trace*=1099511628211ull;}
                    ++pairs;
                }
                // Advance real missile drawing/motion once, then clear so the
                // fixed 50-slot pool cannot hide later emitted pairs.
                missiles.frame=ticks;Missiles.unput_update_render();Missiles.reset();
                if(ticks>400)throw std::runtime_error("Missile pair cycle stalled");
            }
            require(ticks==369 && pairs==35 && eyes.missile_state().frame==0 && eyes.missile_state().subphase==0 && native_random_seed==seed && !player_is_hit,"Complete missile cycle resets prepare state with 35 pairs and one RNG call");
        }
        require(variations[0][0] && variations[0][1] && variations[0][2] && variations[1][0] && variations[1][1],"All original missile variations exercised");
        for(bool clock:{false,true}){
            rank=RANK_NORMAL;native_random_seed=1;player_left=300;player_invincible=true;player_is_hit=false;fb->clear(0,3);fb->clear(1,5);YuugenmaganEyes eyes;eyes.begin_missiles(clock);bool saw_explosion=false;unsigned peak=0;
            for(unsigned tick=1;tick<=369;++tick){
                missiles.frame=tick;Missiles.unput_update_render();eyes.advance_missiles();peak=std::max(peak,Missiles.native_alive_count());
                for(unsigned i=0;i<MISSILE_COUNT;++i)saw_explosion|=Missiles.native_at(i).flag>=MF_HIT;
            }
            require(peak>2 && saw_explosion && !player_is_hit,"Sustained original actor-before-boss updates draw multiple missiles and explosions");
            for(unsigned tick=370;tick<=520;++tick){missiles.frame=tick;Missiles.unput_update_render();}
            require(Missiles.native_alive_count()==0,"All live missiles leave screen, finish explosions and release slots");
        }
        std::cout<<"YuugenMagan southern missile pairs, locked aim, all variation shifts and cycle reset passed; trace "<<std::hex<<trace<<'\n';
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
