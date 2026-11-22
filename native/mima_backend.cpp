#include "mima_backend.hpp"
namespace {
th01_native::MimaBackend* active=nullptr;
th01_native::MimaBackend& backend() {if(!active) throw std::runtime_error("Mima backend is not bound");return *active;}
unsigned current_color=0;
int floor8(int x) {return x>=0 ? x/8*8 : -((-x+7)/8)*8;}
th01_native::Bytes& tile(int id) {return backend().tiles.at(id/64).at(id%64);}
void tile_put(screen_x_t left,screen_y_t top,int id,int quarter=-1) {
    auto& fb=backend().framebuffer;auto& pixels=tile(id);int width=quarter<0 ? 32 : 16;
    int sx=quarter<0 ? 0 : (quarter&1)*16,sy=quarter<0 ? 0 : (quarter>>1)*16;
    int aligned=floor8(left);auto page=th01_native::current_render_page();
    for(int y=0;y<width;++y) for(int x=0;x<width;++x) {int linear=(top+y)*640+aligned+x;if(linear>=0 && linear<640*400) fb.put(page,linear%640,linear/640,pixels.at((sy+y)*32+sx+x));}
}
void tile_snap(screen_x_t left,screen_y_t top,int id) {
    auto& fb=backend().framebuffer;auto& pixels=tile(id);int aligned=floor8(left);
    for(int y=0;y<32;++y) for(int x=0;x<32;++x) {int linear=(top+y)*640+aligned+x;pixels[y*32+x]=linear>=0 && linear<640*400 ? fb.pixel(1,linear%640,linear/640) : 0;}
}
}
namespace th01_native {
MimaBinding::MimaBinding(MimaBackend& value):previous(active) {
    if(!value.wait || !value.delay_ms || !value.music_fade || !value.defeat) throw std::invalid_argument("Mima host callbacks are required");
    if(&value.framebuffer!=&graphics_framebuffer()) throw std::invalid_argument("Mima framebuffer binding mismatch");active=&value;
}
MimaBinding::~MimaBinding() {active=previous;}
}
namespace mima_original {
mima_int_t boss_hp=0,boss_phase_frame=0;std::int8_t boss_phase=0;
void native_mima_reset_backend() {auto& b=backend();b.defeated=false;b.pattern=b.invincibility=0;}
bool native_mima_before_frame() {auto& b=backend();b.missiles.frame=std::int16_t(b.frame);return !b.defeated;}
void native_mima_observe(int pattern,int invincibility) {auto& b=backend();b.pattern=pattern;b.invincibility=invincibility;}
void ptn_new(int slot,int count) {backend().tiles.at(slot).assign(count,th01_native::Bytes(1024,0));}
void ptn_free(int slot) {auto& b=backend();b.tiles.at(slot).clear();if(b.missiles.slot==slot) {b.missiles.sprites={};b.missiles.slot=-1;}}
void ptn_snap_rect_from_1_8(screen_x_t left,screen_y_t top,int w,int h,int slot,int& image,int& x,int& y) {
    page_access(1);for(y=0;y<h/32;++y) for(x=0;x<w/32;++x) {tile_snap(left+x*32,top+y*32,PTN_ID(slot,image));++image;}page_access(0);
}
void ptn_put_row_noalpha_8(screen_x_t left,screen_y_t top,int w,int slot,int& image,int& x) {for(x=0;x<w/32;++x) {tile_put(left+x*32,top,PTN_ID(slot,image));++image;}}
void ptn_put_rect_noalpha_8(screen_x_t left,screen_y_t top,int w,int h,int slot,int& image,int& x,int& y) {for(y=0;y<h/32;++y) ptn_put_row_noalpha_8(left,top+y*32,w,slot,image,x);}
void ptn_put_quarter_noalpha_8(screen_x_t left,screen_y_t top,int id,int quarter) {if(quarter<0 || quarter>3) throw std::invalid_argument("Mima background quarter");tile_put(left,top,id,quarter);}
void graph_r_lineloop_put(const screen_point_t* points,int count,int color) {if(count<1) throw std::invalid_argument("Empty Mima line loop");for(int i=0;i<count;++i) {auto a=points[i],b=points[(i+1)%count];graph_r_line(a.x,a.y,b.x,b.y,color);}}
void graph_r_lineloop_unput(const screen_point_t* points,int count) {if(count<1) throw std::invalid_argument("Empty Mima line loop");for(int i=0;i<count;++i) {auto a=points[i],b=points[(i+1)%count];native_graph_line_unput(a.x,a.y,b.x,b.y);}}
void grcg_setcolor_rmw(int color) {if(color<0 || color>15) throw std::invalid_argument("Mima mono color");current_color=color;}
void grcg_off_func() {}
vram_offset_t vram_offset_shift(int x,int y) {return std::int16_t(y*80+floor8(x)/8);}
void grcg_put(vram_offset_t offset,std::uint32_t mask,int width) {
    auto& fb=backend().framebuffer;auto page=th01_native::current_render_page();
    for(int x=0;x<width;++x) if(mask&((0x80u>>(x&7))<<((x/8)*8))) {int linear=int(offset)*8+x;if(linear>=0 && linear<640*400) fb.put(page,linear%640,linear/640,current_color);}
}
void frame_delay(unsigned frames) {backend().wait(frames);}
void z_palette_white_in() {
    auto& b=backend();const auto target=native_bomb_palette();Palette4 fade;
    for(auto& color:fade.colors) color.set(15,15,15);
    for(unsigned step=0;step<16;++step) {
        b.wait(1);
        for(unsigned color=0;color<16;++color) {
            unsigned argb=0xff000000u;
            for(unsigned component=0;component<3;++component) {auto& value=fade[color].component(component);if(value>target[color].component(component)) --value;argb|=unsigned(value*17)<<(16-component*8);}
            b.framebuffer.palette[color]=argb;
        }
        b.delay_ms(10);
    }
}
void mdrv2_bgm_fade_out_nonblock() {backend().music_fade();}
void boss_defeat_animate() {auto& b=backend();if(!b.defeated) {b.defeated=true;b.defeat();}}
}