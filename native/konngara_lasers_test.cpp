#include "konngara_early.hpp"
#include "laser_backend.hpp"
#include "pellet_backend.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
int main(int argc,char** argv) {
    try {
        auto fb=std::make_unique<Framebuffer>();SpriteSet empty;ShotBackend shots{*fb,empty,{}};ShotBackendBinding binding(shots);
        fb->palette.fill(0xff101010);fb->palette[7]=0xffffffff;
        std::uint64_t trace=1469598103934665603ull;auto mix=[&](unsigned n){trace^=n;trace*=1099511628211ull;};
        for(unsigned difficulty=0;difficulty<4;++difficulty) for(unsigned initial_seed:{1u,4u}) {
            rank=difficulty;native_pellet_speed=0;native_random_seed=initial_seed;player_left=300;player_sliding=false;player_invincible=true;player_is_hit=false;
            for(auto& laser:shootout_lasers)laser={};Pellets.native_reset_all();shots.sound_commands.clear();fb->clear(0,5);fb->clear(1,5);page_access(0);
            std::vector<KonngaraExpression> expressions;KonngaraEarlyPattern pattern(KonngaraEarlyAttack::lasers_spread,[&](auto e){expressions.push_back(e);});
            unsigned ticks=0,volleys=0;auto seed=initial_seed*0x015a4e35u+1;bool reverse=((seed>>16)&0x7fff)%2;
            while(true) {
                ++ticks;bool done=pattern.advance();
                if(ticks==10)require(expressions==std::vector<KonngaraExpression>{KonngaraExpression::aim},"Laser aiming expression on frame ten");
                bool firing=ticks>=100 && ticks%10==0;require(Pellets.native_alive_count()==(firing ? 3 : 0),"Laser fires one three-way spread every ten frames");
                if(firing) {
                    const int delta[]={128,96,80,64};int target=reverse ? 640-int(volleys)*delta[difficulty] : int(volleys)*delta[difficulty];
                    auto& laser=shootout_laser_safe(ticks/10);unsigned char angle=iatan2(330,target-410);
                    require(laser.alive && laser.age==0 && laser.origin_left.v==410*256 && laser.origin_y.v==70*256,"Original laser source and slot schedule");
                    require(laser.step_x.v==Cos8(angle) && laser.step_y.v==Sin8(angle) && laser.ray_extend_speed==8 && laser.ray_moveout_speed==8 && laser.moveout_at_age==30 && laser.width_cel==4 && laser.col==7,"Original laser target, truncation, lifetime and width");
                    for(unsigned i=0;i<3;++i) {auto& p=Pellets.native_at(i);require(p.from_group==PG_3_SPREAD_WIDE_AIMED && p.cur_left.v==410*16 && p.cur_top.v==70*16,"Laser spreads use sword source and original wide aimed group");mix(std::uint16_t(p.velocity.x.v));mix(std::uint16_t(p.velocity.y.v));}
                    ++volleys;Pellets.native_reset_all();
                }
                update_shootout_lasers();for(const auto& plane:fb->pages[0])for(auto byte:plane)mix(byte);
                if(argc>=2 && difficulty==1 && initial_seed==1 && ticks==150)dump_bmp(argv[1],*fb);
                if(done)break;if(ticks>200)throw std::runtime_error("Konngara laser pattern stalled");
            }
            const unsigned totals[]={6,8,9,11};require(volleys==totals[difficulty] && ticks==100+(volleys-1)*10,"Rank-specific laser sweep includes final boundary volley");
            require(native_random_seed==seed && shots.sound_commands==std::vector<unsigned>(volleys,6) && !player_is_hit,"One direction RNG and one SE-6 per volley; invincible live laser updates");
            for(auto& laser:shootout_lasers)laser.unput_and_reset();require(fb->pages[0]==fb->pages[1],"Safe live laser cleanup restores background");
        }
        for(unsigned difficulty=0;difficulty<4;++difficulty) {
            rank=difficulty;native_random_seed=1;Pellets.native_reset_all();shots.sound_commands.clear();unsigned ticks=0,gravity=0,fall=0;std::vector<KonngaraExpression> expressions;
            KonngaraEarlyPattern pattern(KonngaraEarlyAttack::sleeve_rain,[&](auto e){expressions.push_back(e);});
            while(true) {
                ++ticks;bool done=pattern.advance();bool firing=ticks>=100 && ticks%10==0;require(Pellets.native_alive_count()==(firing ? 10 : 0),"Sleeve emits ten projectiles per volley");
                for(unsigned i=0;i<unsigned(Pellets.native_alive_count());++i) {
                    auto& p=Pellets.native_at(i);pixel_t vx,vy;unsigned char angle=0-i*12;vector2(vx,vy,subpixel_t(32+difficulty*6),angle);
                    require(p.cur_left.v==290*16 && p.cur_top.v==200*16 && p.velocity.x.v==vx && p.velocity.y.v==vy,"Sleeve source and original byte-wrapped semicircle angles");
                    bool is_gravity=ticks%20==0;require(p.motion_type==(is_gravity ? PM_GRAVITY : PM_FALL_STRAIGHT_FROM_TOP_THEN_REGULAR) && p.speed.v==(is_gravity ? 1 : 48),"Sleeve alternates gravity and fall-from-top motions");
                    if(is_gravity)++gravity;else ++fall;mix(std::uint16_t(p.velocity.x.v));mix(std::uint16_t(p.velocity.y.v));mix(p.motion_type);
                }
                Pellets.native_reset_all();if(done)break;if(ticks>300)throw std::runtime_error("Sleeve rain stalled");
            }
            require(ticks==300 && gravity==110 && fall==100 && expressions==std::vector<KonngaraExpression>{KonngaraExpression::aim},"Sleeve final frame, alternating totals and expression");
            require(native_random_seed==1 && shots.sound_commands==std::vector<unsigned>(21,7),"Sleeve consumes no RNG and emits 21 SE-7 commands");
        }
        std::cout<<"Konngara lasers/three-way spread and sleeve rain passed across ranks and both sweep directions; trace "<<std::hex<<trace<<'\n';
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
