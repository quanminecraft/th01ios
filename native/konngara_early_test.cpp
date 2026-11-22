#include "konngara_early.hpp"
#include "pellet_backend.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
int main() {
    try {
        auto fb=std::make_unique<Framebuffer>();SpriteSet empty;ShotBackend shots{*fb,empty,{}};ShotBackendBinding binding(shots);
        std::uint64_t trace=1469598103934665603ull;
        for(unsigned difficulty=0;difficulty<4;++difficulty) {
            rank=difficulty;native_pellet_speed=0;native_random_seed=1;player_left=300;player_invincible=true;player_is_hit=false;
            Pellets.native_reset_all();shots.sound_commands.clear();std::vector<KonngaraExpression> expressions;
            {
                KonngaraEarlyPattern pattern(KonngaraEarlyAttack::cup_spreads,[&](auto e) {expressions.push_back(e);});
                for(unsigned tick=1;tick<=300;++tick) {
                    bool done=pattern.advance();require(done==(tick==300),"Cup spread completes at original frame 300");
                    if(tick==10) require(expressions==std::vector<KonngaraExpression>{KonngaraExpression::closed},"Cup closes eyes on frame ten");
                    if(tick==103) require(Pellets.native_alive_count()==0,"Cup warning phase does not fire early");
                    if(tick==104) {
                        require(Pellets.native_alive_count()==2,"First cup volley has two pellets");
                        require(Pellets.native_at(0).cur_left.v==328*16 && Pellets.native_at(1).cur_left.v==296*16 && Pellets.native_at(0).cur_top.v==188*16,"Original cup muzzle coordinates");
                        require(Pellets.native_at(0).velocity.x.v==0 && Pellets.native_at(1).velocity.x.v==0,"Opening cup volley aims vertically");
                        const int expected_speed[]={80,86,108,130};require(Pellets.native_at(0).velocity.y.v==expected_speed[difficulty],"Cup speed includes original pattern and difficulty increments");
                    }
                }
                require(Pellets.native_alive_count()==90 && native_random_seed==1,"Cup generates 90 pellets without RNG");
                auto count=Pellets.native_alive_count();require(pattern.advance() && Pellets.native_alive_count()==count,"Completed pattern is idempotent");
                for(unsigned i=0;i<90;++i) {const auto& p=Pellets.native_at(i);trace^=std::uint16_t(p.velocity.x.v);trace*=1099511628211ull;trace^=std::uint16_t(p.velocity.y.v);trace*=1099511628211ull;}
            }
            Pellets.native_reset_all();shots.sound_commands.clear();expressions.clear();
            fb->clear(0,3);fb->clear(1,5);page_access(0);native_random_seed=1;
            {
                KonngaraEarlyPattern pattern(KonngaraEarlyAttack::diamond_cross_rain,[&](auto e) {expressions.push_back(e);});
                unsigned ticks=0;while(true) {
                    ++ticks;bool done=pattern.advance();
                    if(ticks==10) require(expressions==std::vector<KonngaraExpression>{KonngaraExpression::neutral},"Diamond pattern restores neutral face");
                    if(ticks==99) require(Pellets.native_alive_count()==0,"Diamond warning phase does not spawn pellets");
                    if(ticks==100) require(Pellets.native_alive_count()==(difficulty==0 ? 2 : difficulty==1 ? 3 : 5),"Opening aimed spread respects rank");
                    if(done) break;if(ticks>1000) throw std::runtime_error("Diamond pattern stalled");
                }
                unsigned interval[]={18,16,14,12};unsigned bursts=200/interval[difficulty];
                require(ticks==435,"Diamond movement and 200-frame rain finish at original frame 435 for fixed player position");
                require(Pellets.native_alive_count()==int((difficulty==0 ? 2 : difficulty==1 ? 3 : 5)+bursts*5),"Diamond rain emits five pellets per rank interval");
                std::uint32_t seed=1;for(unsigned i=0;i<bursts*9;++i) seed=seed*0x015a4e35u+1;
                require(native_random_seed==seed,"Rain retains nine coordinate RNG calls per burst");
                require(shots.sound_commands==std::vector<unsigned>{12},"Diamond setup emits one SE-12");
                require(!player_is_hit,"Warning diamonds alone do not add player collision");
                for(auto& plane:fb->pages[0]) for(auto b:plane) {trace^=b;trace*=1099511628211ull;}
                std::cout<<"Rank "<<difficulty<<" diamond completion frame="<<ticks<<'\n';
            }
        }
        std::cout<<"Konngara original diamond rain/cup spreads passed; trace "<<std::hex<<trace<<'\n';
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
