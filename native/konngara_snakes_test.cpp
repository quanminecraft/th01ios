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
        fb->palette.fill(0xff000000);fb->palette[7]=0xffffffff;fb->palette[9]=0xffff00ff;
        std::uint64_t trace=1469598103934665603ull;
        auto mix=[&](unsigned value) {trace^=value;trace*=1099511628211ull;};
        for(unsigned difficulty=0;difficulty<4;++difficulty) for(bool four:{false,true}) {
            rank=difficulty;native_pellet_speed=0;native_random_seed=1;player_left=300;player_sliding=false;player_invincible=true;player_is_hit=false;
            Pellets.native_reset_all();shots.sound_commands.clear();fb->clear(0,0);fb->clear(1,0);page_access(0);
            std::vector<KonngaraExpression> expressions;
            unsigned emissions=0,bursts=0,first_burst=0;
            KonngaraEarlyPattern pattern(four ? KonngaraEarlyAttack::four_snakes : KonngaraEarlyAttack::two_snakes_spreads,[&](auto e) {expressions.push_back(e);});
            unsigned ticks=0;
            while(true) {
                ++ticks;bool done=pattern.advance();
                if(ticks==10) require(expressions==std::vector<KonngaraExpression>{KonngaraExpression::glare},"Snake warning changes face to glare at frame ten");
                if(ticks==99) require(native_random_seed==1 && shots.sound_commands.empty(),"No snake or RNG before frame 100");
                if(ticks==100) require(native_random_seed==0x015a4e36u && shots.sound_commands==std::vector<unsigned>{12},"First snake consumes one wobble RNG call and emits SE-12");
                unsigned count=Pellets.native_alive_count();
                if(count) {require(!four && count==20,"Only paired snakes emit 20-pellet semicircle bursts");++bursts;if(!first_burst)first_burst=ticks;}
                emissions+=count;
                for(unsigned i=0;i<count;++i) {auto& p=Pellets.native_at(i);mix(std::uint16_t(p.cur_left.v));mix(std::uint16_t(p.cur_top.v));mix(std::uint16_t(p.velocity.x.v));mix(std::uint16_t(p.velocity.y.v));}
                // Count each emitted volley without masking it with the 100-slot
                // pool limit. Moving projectile integration is tested separately.
                Pellets.native_reset_all();
                for(const auto& plane:fb->pages[0]) for(auto byte:plane) mix(byte);
                if(argc>=2 && difficulty==1 && ticks==240) dump_bmp(std::string(argv[1])+(four ? "-four.bmp" : "-two.bmp"),*fb);
                if(done) break;if(ticks>500) throw std::runtime_error("Snake pattern stalled");
            }
            require(ticks==(four ? 400u : 500u),"Original snake pattern end frame");
            require(!player_is_hit,"Invincibility suppresses snake hitbox");
            unsigned expected_draws=four ? 900 : 750+6+difficulty;std::uint32_t seed=1;
            for(unsigned i=0;i<expected_draws;++i) seed=seed*0x015a4e35u+1;
            require(native_random_seed==seed,"Original wobble and semicircle RNG call count");
            unsigned se12=0,se7=0;for(auto se:shots.sound_commands) {if(se==12)++se12;else if(se==7)++se7;else throw std::runtime_error("Unexpected snake sound");}
            require(se12==(four ? 4u : 2u) && se7==(four ? 0u : 6+difficulty),"Snake spawn and semicircle sound schedule");
            require(emissions==(four ? 0u : (6+difficulty)*20) && bursts==(four ? 0u : 6+difficulty),"Rank-dependent semicircle volley count");
            if(!four) require(first_burst==280-difficulty*40,"Semicircle rank threshold is strictly greater than original value");
        }
        // Same deterministic motion must hit an unprotected player. Sliding
        // changes the source hitbox and the aiming center, so test both paths.
        for(bool sliding:{false,true}) {
            rank=RANK_NORMAL;native_random_seed=1;player_left=300;player_sliding=sliding;player_invincible=false;player_is_hit=false;
            Pellets.native_reset_all();fb->clear(0,0);fb->clear(1,0);
            KonngaraEarlyPattern pattern(KonngaraEarlyAttack::four_snakes,[](auto) {});
            while(!pattern.advance()) {}
            require(player_is_hit,"Original snake collision hits unprotected player");
        }
        std::cout<<"Konngara two/four snakes: timing, four ranks, RNG, hitboxes and rendering passed; trace "<<std::hex<<trace<<'\n';
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
