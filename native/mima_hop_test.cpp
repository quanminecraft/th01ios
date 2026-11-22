#include "mima_hop.hpp"
#include "boss_entity_backend.hpp"
#include "bomb_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* reason) {if(!ok) throw std::runtime_error(reason);}
int main(int argc,char** argv) {
    try {
        if(argc<2 || argc>3) throw std::invalid_argument("Usage: mima_hop_test ASSET_DIR [BMP]");
        auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();
        auto stg=read_sprites(asset(archive,"STG.PTN")),miko=read_sprites(asset(archive,"MIKO.PTN")),kuji1=read_sprites(asset(archive,"KUZI1.GRC")),kuji2=read_sprites(asset(archive,"KUZI2.GRC"));
        ShotBackend shots{*fb,stg,{}};ShotBackendBinding shot_binding(shots);PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding player_binding(player);
        auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS3.GRP"));fb->palette=bg.palette;
        BombBackend bomb{*fb,kuji1,kuji2,[]() {}};BombBackendBinding bomb_binding(bomb,player);
        BossEntityBackend entities{*fb,archive,{}};BossEntityBinding entity_binding(entities);
        std::uint64_t trace=1469598103934665603ull;
        const int counts[4]={5,8,10,12};const int speeds[4]={30,35,40,45};
        for(unsigned difficulty=0;difficulty<4;++difficulty) {
            rank=difficulty;native_pellet_speed=0;native_random_seed=1;Pellets.native_reset_all();
            for(unsigned page=0;page<2;++page) fb->blit(page,0,0,bg.images[0],false);fb->palette=bg.palette;
            bomb.current=bomb.stage;MimaHopPattern pattern;
            require(native_random_seed==0x015a4e36u,"Hop initialization consumes one direction RNG call");
            unsigned previous=0;
            for(unsigned run=0;run<2;++run) {
                unsigned ticks=0,bursts=0;
                do {
                    ++ticks;bool done=pattern.advance();auto state=pattern.state();
                    if(ticks==1) require(state.hitbox_inactive && !state.meteor,"Hop starts with inactive orb hitbox and meteor disabled");
                    unsigned count=Pellets.native_alive_count();
                    if(count!=previous) {
                        require(count-previous==unsigned(counts[difficulty]),"Each hop fires rank-specific chase count");
                        int left=run==0 ? int(bursts)*128 : 512-int(bursts)*128;
                        for(unsigned i=previous;i<count;++i) {const auto& p=Pellets.native_at(i);require(p.cur_left.to_pixel()==left+64 && p.cur_top.to_pixel()==176 && p.motion_type==PM_CHASE && p.speed.v==speeds[difficulty],"Hop fires chase pellets at original off-center lane and rank speed");}
                        previous=count;++bursts;
                    }
                    for(const auto& page:fb->pages) for(const auto& plane:page) for(auto byte:plane) {trace^=byte;trace*=1099511628211ull;}
                    if(done) break;if(ticks>250) throw std::runtime_error("Hop pattern stalled");
                } while(true);
                auto state=pattern.state();require(ticks==176 && bursts==4 && state.frame==0 && state.meteor && !state.hitbox_inactive,"Four-hop pattern completes after original 176 calls");
                require(fb->palette==bg.palette && native_random_seed==0x015a4e36u,"Hop restores stage palette without further RNG");
                require(state.snapshots==9+run*8,"Hop snapshots each teleport and spread-in start");
                if(run==0) pattern.restart();
            }
        }
        if(argc==3) dump_bmp(argv[2],*fb);
        std::cout<<"Original Mima hop/chase, four ranks, reversed repeat, palette and background trace "<<std::hex<<trace<<" passed\n";
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}