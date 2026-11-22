#include "particle_backend.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* reason) {if(!ok) throw std::runtime_error(reason);}
static unsigned next(std::uint32_t& seed) {seed=seed*0x015a4e35u+1;return (seed>>16)&0x7fff;}
int main() {
    try {
        auto fb=std::make_unique<Framebuffer>();SpriteSet empty;ShotBackend shot{*fb,empty,{}};ShotBackendBinding binding(shot);
        CParticles p{};p.init();native_random_seed=1;std::uint32_t expected_seed=1;
        p.unput_update_render(PO_TOP_RIGHT,7);require(p.spawn_cycle==1 && native_random_seed==1 && !p.alive[0] && !p.alive[1],"First frame skips slot zero without consuming RNG");
        int speed=next(expected_seed)%10+1,y=next(expected_seed)%400;
        p.unput_update_render(PO_TOP_RIGHT,7);
        require(p.alive[1] && p.x[1].v==TO_SP(639)-speed*13 && p.y[1].v==TO_SP(y)+speed*8 && native_random_seed==expected_seed,"Odd slot spawns at right edge with original Q12.4 velocity and two RNG calls");
        require(fb->pixel(0,p.x[1].to_pixel(),p.y[1].to_pixel())==7,"Particle render writes its updated position");
        p={};p.init();p.spawn_cycle=79;native_random_seed=1;expected_seed=1;speed=next(expected_seed)%10+1;int x=next(expected_seed)%640;
        p.unput_update_render(PO_TOP_RIGHT,7);
        require(p.spawn_cycle==speed && p.velocity_base[0]==1 && p.velocity_x[0].v==0 && p.velocity_y[0].v==-speed*13 && p.y[0].v==TO_SP(x)-speed*13 && !p.alive[1] && native_random_seed==expected_seed,"Index forty explicitly reproduces DOS SoA aliases and spawn-cycle corruption");
        p={};p.init();p.spawn_cycle=79;p.velocity_base[0]=1;native_random_seed=5;
        p.unput_update_render(PO_TOP_RIGHT,7);require(p.spawn_cycle==80 && native_random_seed==5,"Nonzero aliased velocity byte suppresses subsequent overflow spawn");
        p.unput_update_render(PO_TOP_RIGHT,7);require(p.spawn_cycle==0 && p.alive[0] && native_random_seed!=5,"Cycle greater than eighty wraps and finally spawns slot zero");
        std::uint64_t trace=1469598103934665603ull;
        for(int origin=0;origin<8;++origin) {
            p={};p.init();native_random_seed=1;fb->clear(0,0);fb->clear(1,5);
            for(int frame=0;frame<400;++frame) {
                p.unput_update_render(static_cast<particle_origin_t>(origin),7);
                for(int i=0;i<PARTICLE_COUNT;++i) {trace^=std::uint16_t(p.x[i].v);trace*=1099511628211ull;trace^=std::uint16_t(p.y[i].v);trace*=1099511628211ull;trace^=p.alive[i];trace*=1099511628211ull;}
                trace^=native_random_seed;trace*=1099511628211ull;
            }
        }
        require(trace==0x17ac28a9671d638bull,"Eight-origin deterministic motion and RNG regression trace");
        p.spawn_cycle=17;p.velocity_base[0]=9;p.init();require(p.spawn_cycle==17 && p.velocity_base[0]==9 && !p.alive[0],"Original init clears alive flags but preserves cycle and stale speeds");
        std::cout<<"Particle spawn, RNG, DOS overflow aliases and eight-origin trace "<<std::hex<<trace<<" passed\n";
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}