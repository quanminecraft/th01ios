#include "mima_pillars.hpp"
#include "pellet_backend.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
static unsigned next(std::uint32_t& seed) {seed=seed*0x015a4e35u+1;return (seed>>16)&0x7fff;}
int main(int argc,char** argv) {
    try {
        auto fb=std::make_unique<Framebuffer>();SpriteSet empty;ShotBackend shots{*fb,empty,{}};ShotBackendBinding binding(shots);
        fb->palette.fill(0xff101010);fb->palette[1]=0xff0000ff;fb->palette[2]=0xffff0000;fb->palette[7]=0xffffffff;
        std::uint32_t expected_seed=1;int centers[8];
        centers[0]=16+next(expected_seed)%48;centers[1]=624-next(expected_seed)%48;
        centers[2]=64+next(expected_seed)%224;centers[3]=336+next(expected_seed)%224;
        centers[4]=128+next(expected_seed)%160;centers[5]=336+next(expected_seed)%160;
        centers[6]=64+next(expected_seed)%224;centers[7]=336+next(expected_seed)%224;
        std::uint64_t trace=1469598103934665603ull;
        for(unsigned difficulty=0;difficulty<4;++difficulty) {
            rank=difficulty;native_pellet_speed=0;native_random_seed=1;Pellets.native_reset_all();shots.sound_commands.clear();player_invincible=true;player_is_hit=false;player_left=centers[0];
            fb->clear(0,3);fb->clear(1,5);page_access(0);std::vector<bool> casting;
            MimaPillarPattern pattern([&](bool cast) {casting.push_back(cast);});unsigned ticks=0;
            do {
                ++ticks;bool done=pattern.advance();
                if(ticks==50) require(casting==std::vector<bool>{true},"Pillar pattern casts at original frame fifty");
                if(ticks==100) require(native_random_seed==expected_seed && shots.sound_commands==std::vector<unsigned>({8,12}),"Pillar setup consumes eight coordinate RNG calls and plays start/circle sounds");
                if(ticks==192) require(!player_is_hit,"Invincibility prevents first pillar collision");
                if(ticks==240) require(casting==std::vector<bool>({true,false}),"Last circle scheduling restores meteor on frame 240");
                if(argc==2 && difficulty==1 && ticks==200) dump_bmp(std::string(argv[1])+"-200.bmp",*fb);
                for(const auto& plane:fb->pages[0]) for(auto byte:plane) {trace^=byte;trace*=1099511628211ull;}
                if(done) break;if(ticks>400) throw std::runtime_error("Pillar pattern stalled");
            } while(true);
            require(ticks==351 && Pellets.native_alive_count()==24 && native_random_seed==expected_seed,"Eight pillars finish with eight aimed three-way spreads and no extra RNG");
            require(shots.sound_commands.size()==17 && !player_is_hit,"Pillar run emits one setup/eight circle/eight pillar sounds without invincible hits");
        }
        rank=RANK_NORMAL;native_random_seed=1;Pellets.native_reset_all();player_invincible=false;player_is_hit=false;player_left=centers[0];
        {
            MimaPillarPattern pattern([](bool) {});for(unsigned frame=1;frame<=192;++frame) {pattern.advance();if(frame<192) require(!player_is_hit,"Pillar collision waits until pillar replaces warning circle");}
            require(player_is_hit,"First pillar hits unprotected player at its original center");
        }
        std::cout<<"Original pillar circles, sprites, hitboxes, RNG and aimed spreads across four ranks passed; trace "<<std::hex<<trace<<'\n';
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}