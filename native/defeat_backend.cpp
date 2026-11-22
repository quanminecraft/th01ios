#include "defeat_backend.hpp"
namespace {
th01_native::DefeatBackend* active=nullptr;
th01_native::DefeatBackend& backend() {if(!active) throw std::runtime_error("Boss defeat backend is not bound");return *active;}
}
namespace th01_native {
DefeatBinding::DefeatBinding(DefeatBackend& value):previous(active) {if(!value.poll) throw std::invalid_argument("Defeat route input is required");active=&value;}
DefeatBinding::~DefeatBinding() {active=previous;}
}
const Palette4& native_defeat_palette() {return backend().post_palette;}
bool native_defeat_skip_route() {return backend().skip_route_selection;}
void native_defeat_route_set(route_t route) {backend().route=route;}
void native_defeat_input(bool16 reset) {th01_native::set_input_keys(backend().poll());input_sense(reset);}
void native_defeat_reset_input() {th01_native::set_input_keys(backend().poll());input_reset_sense();}
void native_defeat_whiteline(screen_y_t y) {auto& fb=backend().framebuffer;for(int x=0;x<640;++x) fb.put(th01_native::current_render_page(),x,y,7);}
void z_graph_clear() {backend().framebuffer.clear(th01_native::current_render_page(),0);}
void player_put_default() {ptn_put_8(player_left,player_top,PTN_MIKO_L);}
void graph_glyphrow_put(int row,int color,const char* text) {
	auto& b=backend();unsigned width=shiftjis_w(text);
	for(int y=row*16;y<row*16+16;++y) for(unsigned x=0;x<width;++x) b.framebuffer.put(th01_native::current_render_page(),x,y,0);
	b.font.shift_jis(b.framebuffer,th01_native::current_render_page(),0,row*16,color,text);
}
void graph_glyphrow_2xscale_1_to_0(screen_x_t left,vram_y_t top,int row,pixel_t width) {
	auto& fb=backend().framebuffer;left=left/8*8;width=width/16*16;
	for(int y=0;y<16;++y) for(int x=0;x<width;++x) for(int yy=0;yy<2;++yy) for(int xx=0;xx<2;++xx)
		fb.put(0,left+x*2+xx,top+y*2+yy,fb.pixel(1,x,row*16+y));
}
