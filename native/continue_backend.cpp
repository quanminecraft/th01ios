#include "continue_backend.hpp"
#include "continue_strings.inc"
namespace {
th01_native::ContinueBackend* active=nullptr;
bool release_required=false;
th01_native::ContinueBackend& backend() {if(!active) throw std::runtime_error("Continue menu is not bound");return *active;}
void native_continue_input(bool16 reset) {
 auto keys=backend().poll();
 if(release_required) {if(!th01_native::any_menu_input(keys))release_required=false;keys={};}
 th01_native::set_input_keys(keys);input_sense(reset);
}
void native_continue_reset() {
 release_required=th01_native::any_menu_input(backend().poll());
 th01_native::set_input_keys({});input_reset_sense();
}
void native_continue_wait(unsigned n) {backend().wait(n);}
void native_continue_use() {++backend().continues;}
void native_continue_stop() {backend().stop_music();}
void native_continue_speed(int max,int delta) {pellet_speed_lower(pellet_speed_t(max),pellet_speed_t(delta));}
void native_continue_text(screen_x_t left,screen_y_t top,int effect,const char* text) {
	auto& b=backend();unsigned width=std::char_traits<char>::length(text)*8;
	if(effect&0x200) for(int y=0;y<16;++y) for(unsigned x=0;x<width;++x) b.framebuffer.put(th01_native::current_render_page(),left+x,top+y,0);
	b.font.shift_jis(b.framebuffer,th01_native::current_render_page(),left,top,effect&~0x200,text);
}
void native_continue_scale(screen_x_t left,screen_y_t top,screen_x_t source_left,screen_y_t source_top,pixel_t width,pixel_t height) {
	auto& fb=backend().framebuffer;left=left/8*8;source_left=source_left/8*8;width=width/16*16;
	for(int y=0;y<height;++y) for(int x=0;x<width;++x) for(int yy=0;yy<2;++yy) for(int xx=0;xx<2;++xx)
		fb.put(0,left+x*2+xx,top+y*2+yy,fb.pixel(1,source_left+x,source_top+y));
}
}
namespace th01_native {
ContinueBinding::ContinueBinding(ContinueBackend& value):previous(active) {
	if(!value.poll || !value.wait || !value.stop_music) throw std::invalid_argument("Continue input/timing/music callbacks are required");active=&value;
}
ContinueBinding::~ContinueBinding() {active=previous;}
}
using th01_vram_colors_t=std::uint8_t;using shiftjis_t=char;
constexpr int V_GRAY=1,GLYPH_FULL_W=16,CONTINUE_TITLE_W=9*16,CONTINUE_CHOICE_W=3*16;
#define FX_CLEAR_BG 0x200
#define graph_putsa_fx native_continue_text
#define graph_2xscale_byterect_1_to_0_slow native_continue_scale
#define input_sense native_continue_input
#define input_reset_sense native_continue_reset
#define frame_delay native_continue_wait
#define resident_continue_use native_continue_use
#define resident (&backend().resident)
#define pellet_speed_lower native_continue_speed
#define mdrv2_bgm_stop native_continue_stop
#include "th01/main/hud/continue_menu.inc"
