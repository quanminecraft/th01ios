#include "totle_transition.hpp"
#include <cstring>
using vram_y_t=std::int16_t;
using uvram_byte_amount_t=std::uint16_t;
using vc2=std::uint8_t;
namespace {
struct Transition {th01_native::Framebuffer& fb;Palette4 target;const std::function<void(unsigned)>& wait;};
Transition* active=nullptr;
constexpr int ROWSHIFT_CHUNK_SIZE=4,ROW_SIZE=80;
void frame_delay(unsigned frames) {if(!active) throw std::runtime_error("TOTLE transition is not bound");active->wait(frames);}
void egc_pagetrans_rowshift_alternating_32(vram_y_t y,uvram_byte_amount_t offset) {
	if(!active || y<0 || y>=400 || offset>76 || offset%4) throw std::out_of_range("Invalid TOTLE row transfer");
	for(unsigned plane=0;plane<4;++plane) {
		auto* dst=active->fb.pages[0][plane].data()+y*80;
		const auto* src=active->fb.pages[1][plane].data()+y*80;
		if(y&1) {std::memmove(dst+4,dst,76);std::memcpy(dst,src+76-offset,4);}
		else {std::memmove(dst,dst+4,76);std::memcpy(dst+76,src+offset,4);}
	}
	page_access(0);
}
void palette_step() {
	for(unsigned color=0;color<16;++color) {
		auto value=native_bomb_palette()[color];for(unsigned comp=0;comp<3;++comp) {auto& c=value.component(comp);auto target=active->target[color].component(comp);if(c!=target) c+=c<target ? 1 : -1;}
		z_palette_set_show(color,value.c.r,value.c.g,value.c.b);
	}
}
}
#define z_palette_step_to(col,comp,pal) palette_step()
#define totle_pagetrans_animate native_totle_pagetrans_animate
#include "th01/main/totle_transition.inc"
namespace th01_native {
void totle_transition(Framebuffer& fb,const SpriteSet& clear_screen,const std::function<void(unsigned)>& wait) {
	if(!wait || clear_screen.images.size()!=1 || clear_screen.images[0].width!=640 || clear_screen.images[0].height!=400) throw std::invalid_argument("TOTLE transition needs a complete screen and presentation callback");
	if(&fb!=&graphics_framebuffer()) throw std::invalid_argument("TOTLE transition must use the bound graphics framebuffer");
	Transition transition{fb,{},wait};for(unsigned i=0;i<16;++i) {auto rgb=clear_screen.palette[i];transition.target[i].set((rgb>>16&255)/17,(rgb>>8&255)/17,(rgb&255)/17);}
	struct Binding {Transition* previous;Binding(Transition& value):previous(active) {active=&value;}~Binding() {active=previous;}} binding(transition);
	page_access(1);fb.blit(1,0,0,clear_screen.images[0],false);page_access(0);native_totle_pagetrans_animate(0);
}
}
