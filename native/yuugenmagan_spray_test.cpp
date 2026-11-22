#include "yuugenmagan_eyes.hpp"
#include "boss_entity_backend.hpp"
#include "pellet_backend.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
    try{
        if(argc!=2)throw std::invalid_argument("Usage: yuugenmagan_spray_test ASSET_DIR");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();SpriteSet empty;ShotBackend shots{*fb,empty,{}};ShotBackendBinding sb(shots);BossEntityBackend entities{*fb,archive,{}};BossEntityBinding eb(entities);std::uint64_t trace=1469598103934665603ull;
        for(unsigned difficulty=0;difficulty<4;++difficulty){
            rank=difficulty;native_pellet_speed=0;native_random_seed=1;player_left=300;player_invincible=true;player_is_hit=false;shots.sound_commands.clear();Pellets.native_reset_all();fb->clear(0,3);fb->clear(1,5);page_access(0);YuugenmaganEyes eyes;eyes.begin_spray();unsigned ticks=0,emissions=0,early_second=0;const unsigned intervals[]={12,8,4,2};
            while(eyes.spray_state().iterations<2){
                ++ticks;eyes.advance_spray();unsigned cycle=(ticks-1)%446+1;auto s=eyes.spray_state();
                bool first=cycle>=100 && cycle<=280;bool second=cycle>=331;
                bool firing=(first && (cycle-100)%intervals[difficulty]==0) || (second && (int(cycle)-346)%int(intervals[difficulty])==0);
                require(Pellets.native_alive_count()==(firing ? 2 : 0),"Original spray cadence includes negative subframes");
                if(cycle==100)require(s.subphase==1 && s.subframe==0,"First spray starts at frame100");
                if(cycle==280)require(s.subphase==2 && s.frame==0,"First spray resets frame after 180 subframes");
                if(cycle==331)require(s.subphase==3 && s.subframe==-15 && s.angle==128,"Second spray resets angle and starts at negative subframe");
                if(firing){
                    unsigned char east=first ? -int(cycle) : 459-int(cycle);
                    for(unsigned i=0;i<2;++i){auto& p=Pellets.native_at(i);pixel_t vx,vy;unsigned char angle=i ? east : 128-east;vector2(vx,vy,subpixel_t(64+difficulty*6),angle);
                        require(p.cur_left.v==(i ? 540 : 92)*16 && p.cur_top.v==160*16,"Original lateral muzzle coordinates");require(p.velocity.x.v==vx && p.velocity.y.v==vy,"Original mirrored byte-wrapped angles and four-pixel base speed");
                        require(p.motion_type==(first ? PM_REGULAR : PM_FALL_STRAIGHT_FROM_TOP_THEN_REGULAR) && p.speed.v==(first ? 0 : 64),"Second spray switches to original fall-from-top motion");
                        trace^=std::uint16_t(p.velocity.x.v);trace*=1099511628211ull;trace^=std::uint16_t(p.velocity.y.v);trace*=1099511628211ull;trace^=p.motion_type;trace*=1099511628211ull;
                    }
                    emissions+=2;if(second && cycle<346)early_second+=2;Pellets.native_reset_all();
                }
                if(cycle==446)require(s.frame==0 && s.subphase==0 && s.angle==0,"Each spray cycle resets frame, subphase and angle");
                if(ticks>892)throw std::runtime_error("Spray cycle stalled");
            }
            const unsigned totals[]={104,148,300,596};const unsigned negative[]={4,4,12,28};require(ticks==892 && emissions==totals[difficulty] && early_second==negative[difficulty],"Two complete cycles retain rank-dependent total and negative-subframe emissions");require(native_random_seed==1 && shots.sound_commands.empty() && !player_is_hit,"Spray consumes no RNG or SE and adds no eye collision");
        }
        std::cout<<"YuugenMagan original mirrored sprays, negative subframes and two-cycle reset passed; trace "<<std::hex<<trace<<'\n';
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
