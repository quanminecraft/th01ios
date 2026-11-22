#include "mima_entrance.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
int main(int argc,char** argv) {
    try {
        if(argc<2 || argc>3) throw std::invalid_argument("Usage: mima_entrance_test ASSET_DIR [BMP_PREFIX]");
        auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto stg=read_sprites(asset(archive,"STG.PTN"));
        ShotBackend shots{*fb,stg,{}};ShotBackendBinding shot_binding(shots);BossEntityBackend entities{*fb,archive,{}};BossEntityBinding entity_binding(entities);
        auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS3.GRP"));fb->palette=bg.palette;for(unsigned p=0;p<2;++p) fb->blit(p,0,0,bg.images[0],false);
        auto original=fb->pages;MimaEntrance entrance;entrance.entity.load("boss3_1.bos",0);entrance.entity.set_image(0);
        require(entrance.entity.h==160 && entrance.entity.vram_w==16,"Real Mima still sprite is 128 by 160");
        unsigned ticks=0;std::uint64_t trace=1469598103934665603ull;
        while(true) {
            bool done=entrance.advance(256,120);++ticks;
            if(ticks<=10) require(fb->pages==original,"Original first ten spread-in iterations do not draw");
            if(ticks==10) require(entrance.snapshots==1 && entrance.entity.cur_left==256 && entrance.entity.cur_top==120,"Frame ten sets final coordinates and snapshots background");
            if(ticks==30 && argc==3) dump_bmp(std::string(argv[2])+"-30.bmp",*fb);
            for(const auto& page:fb->pages) for(const auto& plane:page) for(auto byte:plane) {trace^=byte;trace*=1099511628211ull;}
            if(done) break;if(ticks>60) throw std::runtime_error("Mima entrance stalled");
        }
        require(ticks==51 && entrance.frame==0 && entrance.snapshots==1,"Original initial spread completes on iteration fifty-one");
        const auto& sprite=entities.slots[0].images[0];
        for(int y=0;y<160;++y) for(int x=0;x<128;++x) {
            unsigned i=y*128+x;auto background=entrance.background[i];auto expected=(sprite.opaque[i] ? 0 : background)|sprite.color[i];
            require(fb->pixel(1,256+x,120+y)==expected,"Completion puts real Mima over pristine page-one background");
            if(sprite.opaque[i]) require(fb->pixel(0,256+x,120+y)==sprite.color[i],"Final full sprite replaces every opaque pixel on page zero");
        }
        if(argc==3) dump_bmp(std::string(argv[2])+"-51.bmp",*fb);
        std::cout<<"Original Mima spread-in 51 iterations, background snapshot and both-page sprite passed; trace "<<std::hex<<trace<<'\n';
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}