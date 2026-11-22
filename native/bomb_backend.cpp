#include "bomb_backend.hpp"
#include <utility>
namespace {
th01_native::BombBackend* active=nullptr;
th01_native::BombBackend& backend() {if(!active) throw std::runtime_error("Native bomb backend is not bound");return *active;}
int floor8(int x) {return x>=0 ? x/8 : -((-x+7)/8);}
}
namespace th01_native {
BombBackend::BombBackend(Framebuffer& fb,const SpriteSet& first,const SpriteSet& second,std::function<void()> wait):framebuffer(fb),kuji1(first),kuji2(second),wait_vsync(std::move(wait)) {
	if(!wait_vsync) throw std::invalid_argument("Bomb VSync timing is not bound");
	for(unsigned i=0;i<16;++i) {current[i].c.r=((fb.palette[i]>>16)&255)/17;current[i].c.g=((fb.palette[i]>>8)&255)/17;current[i].c.b=(fb.palette[i]&255)/17;}
	stage=current;
	if(first.images.size()<8 || second.images.empty()) throw std::invalid_argument("Bomb requires eight plus one kuji glyphs");
	for(unsigned i=0;i<9;++i) {auto& image=i<8 ? first.images[i] : second.images[0];if(image.width!=32 || image.height!=32) throw std::invalid_argument("Bomb kuji glyph size");}
}
BombBackendBinding::BombBackendBinding(BombBackend& value,PlayerBackend& target):previous(active),player(target),previous_update(target.bomb_update) {active=&value;target.bomb_update=native_bomb_update;}
BombBackendBinding::~BombBackendBinding() {player.bomb_update=std::move(previous_update);active=previous;}
}
Palette4& native_bomb_palette() {return backend().current;}
const Palette4& native_bomb_stage_palette() {return backend().stage;}
void z_palette_set_show(unsigned color,int r,int g,int b) {
	auto& value=backend().current[color];value.c.r=std::clamp(r,0,15);value.c.g=std::clamp(g,0,15);value.c.b=std::clamp(b,0,15);
	backend().framebuffer.palette[color]=0xff000000u|(unsigned(value.c.r)*17<<16)|(unsigned(value.c.g)*17<<8)|unsigned(value.c.b)*17;
}
void z_palette_set_all_show(const Palette4& palette) {for(unsigned i=0;i<16;++i) z_palette_set_show(i,palette[i].c.r,palette[i].c.g,palette[i].c.b);}
void grc_put_8(screen_x_t left,screen_y_t top,main_grc_slot_t slot,int image,unsigned color) {
	auto& b=backend();const th01_native::SpriteSet* bank=nullptr;
	if(slot==GRC_SLOT_BOMB_KUJI_1) bank=&b.kuji1;else if(slot==GRC_SLOT_BOMB_KUJI_2) bank=&b.kuji2;else throw std::invalid_argument("Unexpected bomb GRC slot");
	auto& sprite=bank->images.at(image);
	if(left< -32 || left>=640 || top< -32 || top>=400) return;
	int row=top*80+floor8(left);
	for(int y=0;y<32;++y) {
		if(row>=0) for(int byte=0;byte<4;++byte) {
			int offset=row+byte;
			if(!((left>=0 && offset/80==row/80) || (left<0 && offset/80!=row/80))) continue;
			for(int bit=0;bit<8;++bit) if(sprite.opaque.at(y*32+byte*8+bit)) {
				int linear=offset*8+bit;if(linear>=0 && linear<640*400) b.framebuffer.put(th01_native::current_render_page(),linear%640,linear/640,color);
			}
		}
		row+=80;if(row>=32000) break;
	}
}
void z_vsync_wait_and_scrollup(screen_y_t top) {auto& b=backend();b.wait_vsync();b.framebuffer.display_scroll_y=(top%400+400)%400;}
void graph_copy_page_to_other(unsigned source) {if(source>1) throw std::out_of_range("Graphics page");auto& fb=backend().framebuffer;fb.pages[1-source]=fb.pages[source];page_access(1-source);}
bool16 graph_invert_32x32_8(screen_x_t left,screen_y_t top,int plane) {
	if(plane<0 || plane>3) throw std::invalid_argument("Unexpected inversion plane");
	if(left<0 || top<0 || left>=608 || top>=368) return true;
	auto& fb=backend().framebuffer;left=floor8(left)*8;
	for(int y=0;y<32;++y) for(int x=0;x<32;++x) fb.put(th01_native::current_render_page(),left+x,top+y,fb.pixel(th01_native::current_render_page(),left+x,top+y)^(1u<<plane));
	return false;
}
