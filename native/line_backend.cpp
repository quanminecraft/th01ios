#include "line_backend.hpp"
#include <algorithm>
namespace {
using vram_offset_t=std::int16_t;using vram_x_t=std::int16_t;using dots16_t=std::uint16_t;
constexpr int RES_X=640,RES_Y=400,ROW_SIZE=80,BYTE_BITS=3,BYTE_MASK=7;
bool graph_r_unput=false;
dots16_t graph_r_pattern=0x80;
th01_native::MimaPoint graph_r_last_line_end{};
vc2 line_color=0;
void grcg_setcolor_rmw(vc2 value) {if(value>15) throw std::invalid_argument("Line color");line_color=value;}
void grcg_off_func() {}
vram_offset_t vram_offset_shift(int x,int y) {return std::int16_t(y*80+(x>=0 ? x/8 : -((-x+7)/8)));}
void grcg_put(vram_offset_t offset,dots16_t mask,int) {
    auto& fb=th01_native::graphics_framebuffer();auto page=th01_native::current_render_page();
    for(int bit=0;bit<16;++bit) if(mask&((0x80u>>(bit&7))<<((bit/8)*8))) {
        int linear=int(offset)*8+bit;if(linear>=0 && linear<640*400) fb.put(page,linear%640,linear/640,line_color);
    }
}
void native_line_restore32(vram_offset_t offset) {
    auto& fb=th01_native::graphics_framebuffer();
    // Host safety boundary for the original real-mode fault at offset -1.
    for(int i=0;i<4;++i) {int at=int(offset)+i;if(at<0 || at>=32000) continue;for(unsigned p=0;p<4;++p) fb.pages[0][p][at]=fb.pages[1][p][at];}
    page_access(0);
}
void graph_r_hline(screen_x_t left,screen_x_t right,vram_y_t y,vc2 color) {
    if(left>right) std::swap(left,right);if(right<0 || left>=640) return;left=std::max<int>(left,0);right=std::min<int>(right,639);
    graph_r_last_line_end={right,y};auto& fb=th01_native::graphics_framebuffer();auto page=th01_native::current_render_page();
    if(color>15) throw std::invalid_argument("Line color");
    for(int x=left;x<=right;++x) {int linear=int(y)*640+x;if(linear>=0 && linear<640*400) fb.put(page,linear%640,linear/640,color);}
}
void graph_r_vline(screen_x_t x,vram_y_t top,vram_y_t bottom,vc2 color) {
    if(top>bottom) std::swap(top,bottom);if(bottom<0 || top>=400) return;top=std::max<int>(top,0);bottom=std::min<int>(bottom,399);
    graph_r_last_line_end={x,bottom};grcg_setcolor_rmw(color);
    dots16_t mask=(graph_r_pattern>>(x&7))|(graph_r_pattern<<(16-(x&7)));
    for(int y=top;y<=bottom;++y) grcg_put(vram_offset_shift(x,y),mask,16);
}
}
#include "th01/hardware/graph_r_line.inc"
void graph_r_line_patterned(screen_x_t left,vram_y_t top,screen_x_t right,vram_y_t bottom,vc2 color,std::uint16_t pattern) {
    graph_r_pattern=pattern;
    try {graph_r_line(left,top,right,bottom,color);graph_r_pattern=0x80;}catch(...) {graph_r_pattern=0x80;throw;}
}
void native_graph_line_unput(screen_x_t left,vram_y_t top,screen_x_t right,vram_y_t bottom) {
    graph_r_unput=true;
    try {graph_r_line(left,top,right,bottom,7);graph_r_unput=false;}catch(...) {graph_r_unput=false;throw;}
}
namespace th01_native {
void render_mima_square(bool restore,const std::array<MimaPoint,4>& points) {
    for(unsigned i=0;i<4;++i) {auto a=points[i],b=points[(i+1)%4];if(restore) native_graph_line_unput(a.x,a.y,b.x,b.y);else graph_r_line(a.x,a.y,b.x,b.y,7);}
}
}
