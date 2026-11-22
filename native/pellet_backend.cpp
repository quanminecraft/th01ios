#include "pellet_backend.hpp"
#include "pellet_sprites.inc"

std::int16_t native_pellet_speed=0;
namespace {
void plot(int left,int top,const std::uint8_t* dots,int width,int height,unsigned color,bool restore) {
	auto& fb=th01_native::graphics_framebuffer();
	for(int y=0;y<height;++y) for(int x=0;x<width;++x) {
		if(!(dots[y*(width/8)+x/8]&(0x80u>>(x%8)))) continue;
		int linear=(top+y)*640+left+x;
		if(linear<0 || linear>=640*400) continue;
		int dx=linear%640,dy=linear/640;
		fb.put(restore ? 0 : th01_native::current_render_page(),dx,dy,restore ? fb.pixel(1,dx,dy) : color);
	}
	if(restore) page_access(0);
}
}
void native_pellet_put(screen_x_t left,screen_y_t top,int cel) {
	if(cel<0 || cel>=3) throw std::runtime_error("Invalid pellet animation cel");
	plot(left,top,native_pellet_dots[cel],8,8,V_WHITE,false);
}
void pellet_cloud_put_8(screen_x_t left,screen_y_t top,unsigned color,int cel) {
	if(cel<0 || cel>=2 || color>15) throw std::runtime_error("Invalid pellet cloud");
	left=left>=0 ? left/8*8 : -((-left+7)/8)*8;
	plot(left,top,native_cloud_dots[cel],16,16,color,false);
}
void pellet_cloud_unput_8(screen_x_t left,screen_y_t top,int cel) {
	if(cel<0 || cel>=2) throw std::runtime_error("Invalid pellet cloud cel");
	left=left>=0 ? left/8*8 : -((-left+7)/8)*8;
	plot(left,top,native_cloud_dots[cel],16,16,0,true);
}
