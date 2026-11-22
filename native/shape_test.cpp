#include "shot_backend.hpp"
#include "th01/main/shape.hpp"
#include "th01/sprites/shape8x8.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* reason) {if(!ok) throw std::runtime_error(reason);}
int main() {
    try {
        auto fb=std::make_unique<Framebuffer>();SpriteSet empty;ShotBackend shot{*fb,empty,{}};ShotBackendBinding binding(shot);
        using Put=void(*)(screen_x_t,vram_y_t,vc2);
        std::array<Put,3> puts{shape8x8_diamond_put,shape8x8_star_put,shape8x8_flake_put};
        for(int shape=0;shape<3;++shape) for(int shift=0;shift<8;++shift) {
            fb->clear(0,3);fb->clear(1,5);page_access(1);puts[shape](104+shift,100,6);
            for(int y=99;y<109;++y) for(int x=96;x<128;++x) {
                int sx=x-104-shift,sy=y-100;bool dot=sx>=0 && sx<8 && sy>=0 && sy<8 && (sSHAPE8X8[shape][sy]&(0x80u>>sx));
                require(fb->pixel(1,x,y)==(dot ? 6 : 5) && fb->pixel(0,x,y)==3,"Shape draws exact mask/color on selected page at every pixel shift");
            }
        }
        auto before=fb->pages;shape8x8_star_put(-1,100,2);shape8x8_star_put(640,100,2);shape8x8_star_put(100,400,2);require(fb->pages==before,"Shapes reject out-of-range origin");
        page_access(0);fb->clear(0,0);shape_ellipse_arc_put(100,100,20,20,7,64,0,128);
        require(fb->pixel(0,120,100)==7 && fb->pixel(0,100,120)==7 && fb->pixel(0,80,100)==0,"Arc flushes earlier bytes but preserves original missing final cache flush");
        fb->clear(0,0);before=fb->pages;shape_ellipse_arc_put(100,100,20,20,7,1,64,64);require(fb->pages==before,"Equal-angle arc draws nothing");
        shape_ellipse_arc_put(100,100,20,20,7,1,128,0);require(fb->pages==before,"Reversed angle arc draws nothing");
        fb->clear(0,3);fb->clear(1,5);shape_ellipse_arc_sloppy_unput(100,100,20,20,1,0,0);
        for(int x=100;x<140;++x) require(fb->pixel(0,x,100)==(x>=112 && x<128 ? 5 : 3),"Arc unput restores aligned sixteen-pixel band even for a single angle");
        before=fb->pages;shape_ellipse_arc_sloppy_unput(100,100,20,20,1,128,0);require(fb->pages==before,"Reversed unput arc does not restore");
        std::cout<<"Native diamond/star/flake masks, pixel offsets, page selection and shared original arc cache passed\n";
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}