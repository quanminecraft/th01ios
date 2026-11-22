#include "konngara_early.hpp"
#include "pellet_backend.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
int main() {
    try {
        auto fb=std::make_unique<Framebuffer>();SpriteSet empty;ShotBackend shots{*fb,empty,{}};ShotBackendBinding binding(shots);
        std::uint64_t trace=1469598103934665603ull;auto mix=[&](unsigned n) {trace^=n;trace*=1099511628211ull;};
        for(unsigned difficulty=0;difficulty<4;++difficulty) for(bool spray:{false,true}) {
            rank=difficulty;native_pellet_speed=0;native_random_seed=1;player_left=300;player_sliding=false;player_invincible=true;player_is_hit=false;
            Pellets.native_reset_all();shots.sound_commands.clear();fb->clear(0,3);fb->clear(1,5);page_access(0);
            std::vector<KonngaraExpression> expressions;unsigned ticks=0,emissions=0;bool rows[3]={};
            KonngaraEarlyPattern pattern(spray ? KonngaraEarlyAttack::cup_spray : KonngaraEarlyAttack::aimed_rows,[&](auto e) {expressions.push_back(e);});
            while(true) {
                ++ticks;bool done=pattern.advance();
                if(ticks==10) require(expressions==std::vector<KonngaraExpression>{spray ? KonngaraExpression::closed : KonngaraExpression::neutral},"Original frame-ten expression change");
                if(ticks<100) require(Pellets.native_alive_count()==0,"Warning phase does not emit projectiles");
                if(Pellets.native_alive_count()) {
                    require(Pellets.native_alive_count()==1,"One projectile per firing tick");auto& p=Pellets.native_at(0);
                    pixel_t vx=0,vy=0;
                    if(spray) {
                        const unsigned interval[]={5,4,3,2};require(ticks%interval[difficulty]==0,"Cup spray rank firing cadence");
                        require(p.cur_left.v==312*16 && p.cur_top.v==188*16,"Cup spray muzzle coordinates");
                        if(emissions<10) {
                            const unsigned offsets[]={32,24,16,8,0,248,240,232,224,216};
                            unsigned char angle=iatan2(player_top-184,player_left-278)+offsets[emissions];
                            vector2(vx,vy,subpixel_t(48+difficulty*6),angle);
                            require(p.velocity.x.v==vx && p.velocity.y.v==vy,"Original offset aiming and ninth-shot direction reversal");
                        }
                    } else {
                        const unsigned interval[]={12,10,8,6};require(ticks%interval[difficulty]==0,"Row rank firing cadence");
                        int x=p.cur_left.to_pixel(),y=p.cur_top.to_pixel();require(x>=64 && x<=576 && (y==64 || y==128 || y==192),"Diamond follows three horizontal rows without firing during vertical travel");rows[(y-64)/64]=true;
                        vector2_between(pixel_t(x),pixel_t(y),pixel_t(player_center_x()-PELLET_W),player_center_y(),vx,vy,subpixel_t(48+emissions*2+difficulty*6));
                        require(p.velocity.x.v==vx && p.velocity.y.v==vy,"Each row shot increases base speed by two subpixels");
                    }
                    ++emissions;mix(ticks);mix(std::uint16_t(p.cur_left.v));mix(std::uint16_t(p.cur_top.v));mix(std::uint16_t(p.velocity.x.v));mix(std::uint16_t(p.velocity.y.v));
                    Pellets.native_reset_all();
                }
                if(!spray) for(const auto& plane:fb->pages[0]) for(auto byte:plane) mix(byte);
                if(done) break;if(ticks>800) throw std::runtime_error("Rows/spray pattern stalled");
            }
            require(native_random_seed==1 && !player_is_hit,"Rows/spray do not consume RNG or add diamond hitbox");
            if(spray) {const unsigned totals[]={121,151,200,301};require(ticks==700 && emissions==totals[difficulty] && shots.sound_commands.empty(),"Spray total includes final frame and no extra SE");}
            else {const unsigned totals[]={18,20,24,33};require(ticks==335 && emissions==totals[difficulty],"Original row completion frame and rank-dependent totals");require(rows[0] && rows[1] && rows[2] && shots.sound_commands==std::vector<unsigned>{12},"All three rows and one setup SE");std::cout<<"Row rank "<<difficulty<<" ticks="<<ticks<<" emissions="<<emissions<<'\n';}
        }
        std::cout<<"Konngara original rows/spray across four ranks passed; trace "<<std::hex<<trace<<'\n';
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
