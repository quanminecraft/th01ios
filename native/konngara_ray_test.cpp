#include "konngara_early.hpp"
#include "pellet_backend.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
int main(int argc,char** argv) {
    try {
        auto fb=std::make_unique<Framebuffer>();SpriteSet empty;ShotBackend shots{*fb,empty,{}};ShotBackendBinding binding(shots);
        std::uint64_t trace=1469598103934665603ull;auto mix=[&](unsigned n) {trace^=n;trace*=1099511628211ull;};
        for(unsigned difficulty=0;difficulty<4;++difficulty) {
            rank=difficulty;native_pellet_speed=0;native_random_seed=1;player_left=300;player_invincible=true;player_is_hit=false;
            Pellets.native_reset_all();shots.sound_commands.clear();fb->clear(0,5);fb->clear(1,5);page_access(0);
            fb->palette.fill(0xff000000);fb->palette[5]=0xff202020;fb->palette[6]=0xffffff00;
            std::vector<KonngaraExpression> expressions;unsigned ticks=0,emissions=0;std::uint32_t seed=1;
            KonngaraEarlyPattern pattern(KonngaraEarlyAttack::rain_edges,[&](auto e) {expressions.push_back(e);});
            while(true) {
                ++ticks;bool done=pattern.advance();
                if(ticks==10) require(expressions==std::vector<KonngaraExpression>{KonngaraExpression::aim},"Ray changes face to aiming expression");
                if(ticks<100) require(fb->pages[0]==fb->pages[1] && shots.sound_commands.empty() && native_random_seed==1,"Ray warning phase is inert");
                unsigned interval[]={5,3,2,2};bool firing=ticks>=100 && (ticks==360 || ticks%interval[difficulty]==0);
                require(Pellets.native_alive_count()==(firing ? 2 : 0),"Original paired rain cadence including final reset-frame volley");
                if(firing) {
                    int x=0,y=0;
                    if(ticks<125) y=256-8*(ticks-100);
                    else if(ticks<205) {x=8*(ticks-124);y=64;}
                    else if(ticks<230) {x=639;y=64+8*(ticks-204);}
                    else if(ticks<255) {x=639;y=264-8*(ticks-229);}
                    else if(ticks<335) {x=639-8*(ticks-254);y=64;}
                    else if(ticks<360) y=64+8*(ticks-334);
                    else y=256;
                    for(unsigned i=0;i<2;++i) {
                        const auto& p=Pellets.native_at(i);require(p.cur_left.v==x*16 && p.cur_top.v==y*16,"Rain spawns after endpoint movement on original path");
                        require(p.motion_type==PM_GRAVITY && p.speed.v==1,"Rain carries original one-subpixel gravity acceleration");
                        seed=seed*0x015a4e35u+1;unsigned char angle=((seed>>16)&0x7fff)&0x7f;pixel_t vx,vy;vector2(vx,vy,subpixel_t(32+difficulty*6),angle);
                        require(p.velocity.x.v==vx && p.velocity.y.v==vy,"Rain random angle and rank speed");
                        mix(std::uint16_t(p.cur_left.v));mix(std::uint16_t(p.cur_top.v));mix(std::uint16_t(p.velocity.x.v));mix(std::uint16_t(p.velocity.y.v));
                    }
                    emissions+=2;Pellets.native_reset_all();
                }
                if(argc>=2 && difficulty==1 && ticks==180) dump_bmp(argv[1],*fb);
                for(const auto& plane:fb->pages[0]) for(auto b:plane) mix(b);
                if(done) break;if(ticks>360) throw std::runtime_error("Sword ray stalled");
            }
            unsigned totals[]={106,174,262,262};require(ticks==360 && emissions==totals[difficulty],"Original ray duration and total emissions");
            require(native_random_seed==seed && shots.sound_commands==std::vector<unsigned>(27,6),"Two RNG calls per volley and final-frame SE");
            require(fb->pages[0]==fb->pages[1] && !player_is_hit,"Final line restoration leaves background and player intact");
            auto sound_count=shots.sound_commands.size();require(pattern.advance() && shots.sound_commands.size()==sound_count,"Completed ray does not emit again");
        }
        std::cout<<"Konngara sword ray path, rain/gravity, final-frame behavior and background restore passed; trace "<<std::hex<<trace<<'\n';
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
