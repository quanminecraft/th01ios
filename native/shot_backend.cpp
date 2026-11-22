#include "shot_backend.hpp"

namespace {
th01_native::ShotBackend* active=nullptr;
th01_native::ShotBackend& backend() {
	if(!active) throw std::runtime_error("Shot rendering/audio backend is not bound");
	return *active;
}
int floor_to(int value,int alignment) {
	return value>=0 ? value/alignment*alignment : -((-value+alignment-1)/alignment)*alignment;
}
// VRAM accesses at negative or overflowing X wrap horizontally in the
// upstream EGC/PTN routines. Keep that behavior within valid page memory.
bool address(int left,int top,int& x,int& y) {
	int linear=top*640+left;
	if(linear<0 || linear>=640*400) return false;
	x=linear%640;y=linear/640;return true;
}
void quarter_op(screen_x_t left,screen_y_t top,int id,int quarter,bool restore) {
	auto& b=backend();
	if(quarter<0 || quarter>3) throw std::runtime_error("Invalid PTN quarter");
	auto& sprite=b.sprites.images.at(id);
	if(sprite.width!=32 || sprite.height!=32) throw std::runtime_error("PTN quarter requires 32x32 sprite");
	int start_x=(quarter&1)*16,start_y=(quarter>>1)*16;
	left=floor_to(left,8);
	for(int y=0;y<16;++y) for(int x=0;x<16;++x) {
		auto src=(start_y+y)*32+start_x+x;
		int dest_x,dest_y;
		if(!sprite.opaque.at(src) || !address(left+x,top+y,dest_x,dest_y)) continue;
		auto color=restore ? b.framebuffer.pixel(1,dest_x,dest_y) : sprite.color.at(src);
		b.framebuffer.put(restore ? 0 : b.page,dest_x,dest_y,color);
	}
	if(restore) b.page=0;
}
}
namespace th01_native {
unsigned current_render_page() {return backend().page;}
Framebuffer& graphics_framebuffer() {return backend().framebuffer;}
ShotBackendBinding::ShotBackendBinding(ShotBackend& value):previous(active) {active=&value;}
ShotBackendBinding::~ShotBackendBinding() {active=previous;}
}
void ptn_sloppy_unput_quarter_16(screen_x_t left,screen_y_t top) {
	egc_copy_rect_1_to_0_16(left,top,16,16);
}
void ptn_sloppy_unput_16(screen_x_t left,screen_y_t top) {egc_copy_rect_1_to_0_16(left,top,32,32);}
void egc_copy_rect_1_to_0_16_word_w(screen_x_t left,screen_y_t top,pixel_t width,pixel_t height) {egc_copy_rect_1_to_0_16(left,top,(width/16)*16+16,height);}
void egc_copy_rect_1_to_0_16(screen_x_t left,screen_y_t top,pixel_t width,pixel_t height) {
	auto& fb=backend().framebuffer;
	int start=floor_to(left,16),end=floor_to(left+width+15,16);
	for(int y=0;y<height;++y) for(int x=start;x<end;++x) {
		int dest_x,dest_y;
		if(address(x,top+y,dest_x,dest_y)) fb.put(0,dest_x,dest_y,fb.pixel(1,dest_x,dest_y));
	}
	backend().page=0;
}
void page_access(unsigned page) {
	if(page>=2) throw std::runtime_error("Invalid framebuffer page");
	backend().page=page;
}
void ptn_put_quarter_8(screen_x_t left,screen_y_t top,int id,int quarter) {quarter_op(left,top,id,quarter,false);}
void ptn_unput_quarter_8(screen_x_t left,screen_y_t top,int id,int quarter) {quarter_op(left,top,id,quarter,true);}
void mdrv2_se_play(int id) {
	if(id<0 || id>255) throw std::runtime_error("Invalid sound effect ID");
	backend().sound_commands.push_back(static_cast<unsigned>(id));
 if(backend().sound_dispatch)backend().sound_dispatch(static_cast<unsigned>(id));
}
