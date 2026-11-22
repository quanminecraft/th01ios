#include "shot_backend.hpp"
#include "th01/main/shape.hpp"
#include "th01/sprites/shape8x8.hpp"
#include "th01/math/polar.hpp"
namespace {
using dots8_t=std::uint8_t;using vram_offset_t=std::int16_t;
constexpr int RES_Y=400,PLANE_SIZE=32000,BYTE_MASK=7,BYTE_BITS=3,BYTE_DOTS=8;
std::uint8_t arc_color=0;
void grcg_setcolor_rmw(vc2 color) {if(color>15) throw std::invalid_argument("Shape color");arc_color=color;}
void grcg_off_func() {}
vram_offset_t vram_offset_shift(int left,int top) {return std::int16_t(top*80+(left>=0 ? left/8 : -((-left+7)/8)));}
void grcg_put(vram_offset_t offset,dots8_t dots,int) {
    auto& fb=th01_native::graphics_framebuffer();auto page=th01_native::current_render_page();
    for(int x=0;x<8;++x) if(dots&(0x80u>>x)) {int linear=int(offset)*8+x;if(linear>=0 && linear<640*400) fb.put(page,linear%640,linear/640,arc_color);}
}
void put_shape(int shape,int left,int top,vc2 color) {
    if(left<0 || left>=640 || top<0 || top>=400) return;
    if(color>15) throw std::invalid_argument("Shape color");
    auto& fb=th01_native::graphics_framebuffer();auto page=th01_native::current_render_page();
    for(int y=0;y<8;++y) for(int x=0;x<8;++x) if(sSHAPE8X8[shape][y]&(0x80u>>x)) {
        int linear=(top+y)*640+left+x;
        if(linear<640*400) fb.put(page,linear%640,linear/640,color);
    }
}
}
void shape8x8_diamond_put(screen_x_t x,vram_y_t y,vc2 c) {put_shape(SHAPE8X8_DIAMOND,x,y,c);}
void shape8x8_star_put(screen_x_t x,vram_y_t y,vc2 c) {put_shape(SHAPE8X8_STAR,x,y,c);}
void shape8x8_flake_put(screen_x_t x,vram_y_t y,vc2 c) {put_shape(SHAPE8X8_FLAKE,x,y,c);}
// Keep the original cached-byte arc loop, including its unflushed last byte.
#include "th01/main/shape_arc.inc"
#include "th01/main/shape_arc_unput.inc"
