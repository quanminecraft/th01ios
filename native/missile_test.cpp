#include "missile_backend.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool condition,const char* reason) {if(!condition) throw std::runtime_error(reason);}
int main(int argc,char** argv) {
    try {
        if(argc!=2) throw std::invalid_argument("Usage: missile_test ASSET_DIR");
        auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto stg=read_sprites(asset(archive,"STG.PTN"));
        ShotBackend shot{*fb,stg,{}};ShotBackendBinding shot_binding(shot);
        MissileBackend b{archive};MissileBinding binding(b);Missiles.reset();Missiles.load(PTN_SLOT_BOSS_1);
        require(b.sprites.images.size()>=5,"Real missile bank has moving and explosion cels");
        player_invincible=true;player_is_hit=false;fb->clear(0,3);fb->clear(1,5);
        Missiles.add(200,100,1.5,0);b.frame=0;Missiles.unput_update_render();
        auto& sprite=b.sprites.images[0];
        for(int y=0;y<16;++y) for(int x=0;x<16;++x) {auto i=y*32+x;require(fb->pixel(0,201+x,100+y)==(sprite.opaque[i] ? sprite.color[i] : 3),"Missiles render at pixel-precise subpixel position");}
        // Exercise every directional cel and every pixel alignment independently
        // of motion, including pixels carried into the third destination byte.
        for(int cel=0;cel<16;++cel) for(int shift=0;shift<8;++shift) {
            fb->clear(0,3);page_access(0);native_missile_put(200+shift,100,PTN_ID(PTN_SLOT_BOSS_1,cel/4),cel%4);
            const auto& image=b.sprites.images[cel/4];int quarter=cel%4;
            for(int y=0;y<16;++y) for(int x=-1;x<24;++x) {
                unsigned expected=3;
                if(x>=0 && x<16) {auto i=((quarter>>1)*16+y)*32+(quarter&1)*16+x;if(image.opaque[i]) expected=image.color[i];}
                if(fb->pixel(0,200+shift+x,100+y)!=expected) throw std::runtime_error("Missile cel="+std::to_string(cel)+" shift="+std::to_string(shift)+" x="+std::to_string(x)+" y="+std::to_string(y)+" expected="+std::to_string(expected)+" actual="+std::to_string(fb->pixel(0,200+shift+x,100+y)));
            }
        }
        // Restore the first moving missile's rendered frame for interlace check.
        fb->clear(0,3);native_missile_put(201,100,PTN_ID(PTN_SLOT_BOSS_1,0),0);
        auto before=fb->pages;b.frame=1;Missiles.unput_update_render();require(fb->pages==before,"Opposite interlace field does not draw or unput missile zero");
        b.frame=2;Missiles.unput_update_render();
        require(fb->pixel(0,192,100)==5,"Rendered field restores original aligned sloppy background");
        Missiles.reset();player_invincible=false;player_is_hit=false;
        for(int i=0;i<MISSILE_COUNT;++i) Missiles.add(200,100,0,0);
        Missiles.add(player_left,player_top,0,0);b.frame=0;Missiles.unput_update_render();require(!player_is_hit,"Full pool rejects a fifty-first missile");
        Missiles.reset();Missiles.add(player_left,player_top,0,0);Missiles.unput_update_render();require(player_is_hit,"Moving missile uses original player collision");
        player_is_hit=false;player_invincible=true;Missiles.unput_update_render();require(!player_is_hit,"Invincibility suppresses missile collision");
        Missiles.reset();shot.sound_commands.clear();Missiles.add(0,200,-1,0);
        for(int frame=0;frame<=4;++frame) {b.frame=frame;Missiles.unput_update_render();}
        require(shot.sound_commands==std::vector<unsigned>{7},"Boundary explosion advances on matching fields and plays SE 7 once");
        auto finished=fb->pages;b.frame=6;Missiles.unput_update_render();require(fb->pages==finished && shot.sound_commands.size()==1,"Expired missile releases its slot and stops drawing");
        Missiles.reset();std::cout<<"Original missile motion, pixel rendering, interlace, pool, collision and explosion passed\n";
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}