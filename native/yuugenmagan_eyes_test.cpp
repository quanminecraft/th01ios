#include "yuugenmagan_eyes.hpp"
#include "boss_entity_backend.hpp"
#include "player_backend.hpp"
#include "th01/main/player/player.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
    try{
        if(argc<2)throw std::invalid_argument("Usage: yuugenmagan_eyes_test ASSET_DIR [BMP]");
        auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS2.GRP"));fb->palette=bg.palette;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);
        SpriteSet empty;ShotBackend shots{*fb,empty,{}};ShotBackendBinding sb(shots);BossEntityBackend entities{*fb,archive,{}};BossEntityBinding eb(entities);
        std::uint64_t trace=1469598103934665603ull;
        {
            YuugenmaganEyes eyes;int lefts[]={64,512,192,384,288},tops[]={128,128,176,176,64};
            require(entities.slots[0].images.size()==7,"Seven original eye cels share one BOS bank");
            for(unsigned i=0;i<5;++i){auto e=eyes.eye(i);require(e.left==lefts[i] && e.top==tops[i] && e.cel==0 && !e.hitbox_active,"Original five-eye positions, hidden images and inactive initial hitboxes");}
            eyes.set_cel(31,3);
            for(unsigned i=0;i<5;++i){player_left=lefts[i];eyes.track_and_render(1u<<i);require(eyes.eye(i).cel==3,"Exact left pupil threshold looks down");player_left=lefts[i]-1;eyes.track_and_render(1u<<i);require(eyes.eye(i).cel==4,"Beyond left threshold looks left");player_left=lefts[i]+64;eyes.track_and_render(1u<<i);require(eyes.eye(i).cel==3,"Exact right threshold looks down");player_left=lefts[i]+65;eyes.track_and_render(1u<<i);require(eyes.eye(i).cel==5,"Beyond right threshold looks right");}
            auto before=fb->pages;eyes.track_and_render(0);require(fb->pages==before,"Zero selection does not redraw any eye");
            player_left=300;eyes.set_cel(31,6);for(unsigned n=0;n<8;++n)eyes.track_and_render();
            require(eyes.eye(0).cel==5 && eyes.eye(1).cel==4 && eyes.eye(4).cel==3,"Five eyes independently track the same player");
            if(argc>=3)dump_bmp(argv[2],*fb);
            for(auto& plane:fb->pages[0])for(auto b:plane){trace^=b;trace*=1099511628211ull;}
        }
        require(entities.slots[0].images.empty(),"Eye controller frees shared BOS bank");
        std::cout<<"YuugenMagan real-eye setup, tracking thresholds, flag selection and rendering passed; trace "<<std::hex<<trace<<'\n';
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
