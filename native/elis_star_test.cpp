#include "elis_teleport.hpp"
#include "boss_entity_backend.hpp"
#include "pellet_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
 try{
    if(argc<2)throw std::invalid_argument("Usage: elis_star_test ASSET_DIR [BMP]");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS5.GRP"));fb->palette=bg.palette;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);
    auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);BossEntityBackend entities{*fb,archive,{}};BossEntityBinding eb(entities);page_access(0);ElisTeleport elis(3);
    std::uint64_t trace=1469598103934665603ull;
    for(unsigned cycle=0;cycle<2;++cycle){
        shots.sound_commands.clear();auto seed=native_random_seed;elis.begin_star();
        for(unsigned tick=1;tick<=132;++tick){require(elis.advance_star()==(tick==132),"Original star summon/flash lasts 132 ticks");require(native_random_seed==seed,"Star animation consumes no RNG");
            if(argc>2 && cycle==0 && tick==81)dump_bmp(argv[2],*fb);
            if(tick==81 || tick==97)require(fb->pixel(0,320,210)==7,"Original six-point star white chord");
            if(tick==95)require(fb->pixel(0,320,210)==4,"Original star flash uses FX palette index");

            for(auto& plane:fb->pages[0])for(auto b:plane){trace^=b;trace*=1099511628211ull;}
        }
        require(elis.frame()==0 && shots.sound_commands.size()==1 && shots.sound_commands[0]==8,"Star completion resets clock and emits only summon SE8");
        require(fb->pixel(0,320,210)==fb->pixel(1,320,210),"Final unput restores star chord from background page");auto before=fb->pages;require(elis.advance_star() && before==fb->pages,"Completed star is inert");
        elis.begin_human_pattern(2);for(unsigned i=0;i<61;++i)elis.advance_human_pattern();
    }
    for(unsigned cycle=0;cycle<2;++cycle){
        elis.begin_phase3(1);shots.sound_commands.clear();
        for(unsigned tick=1;tick<=193;++tick){require(elis.advance_phase3()==(tick==193),"Original phase-three controller runs 132 star ticks then 61 ring ticks");require(native_random_seed==3,"Phase-three fixed ring selection consumes no RNG");}
        require(elis.frame()==0 && shots.sound_commands.size()==1 && shots.sound_commands[0]==8,"Phase-three completion clears pattern and keeps star-only sound");
    }    elis.entrance(archive,[](unsigned){});
    for(unsigned attack:{2u,3u}){elis.begin_phase3(attack);unsigned duration=132+(attack==2 ? 251:221);for(unsigned tick=1;tick<=duration;++tick){Pellets.native_reset_all();require(elis.advance_phase3()==(tick==duration),"Phase-three controller hands star off to complete sphere/rift attack");}require(elis.frame()==0,"Phase-three alternate attack resets clock");}    std::cout<<"Elis original six-point star, 132-tick summon/flash, cleanup and repeated attack handoff passed; trace "<<std::hex<<trace<<'\n';
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}