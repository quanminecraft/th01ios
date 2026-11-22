#include "laser_backend.hpp"
#include "laser_sprites.inc"
std::uint16_t native_laser_mask(unsigned index) {
	if(index>=64) throw std::out_of_range("Laser mask index");
	return native_laser_dots[index];
}
void native_laser_put(screen_x_t left,vram_y_t top,dots16_t mask,vc2 color) {
	auto& fb=th01_native::graphics_framebuffer();int start=left>=0 ? left/8*8 : -((-left+7)/8)*8;
	for(int bit=0;bit<16;++bit) if(mask&((0x80u>>(bit&7))<<((bit/8)*8))) {
		int linear=top*640+start+bit;
		if(linear>=0 && linear<640*400) fb.put(th01_native::current_render_page(),linear%640,linear/640,color);
	}
}
void native_laser_reset_restore(CShootoutLaser& laser) {
	// Native repair for upstream's Q24.8-to-int16 endpoint bug. Restore
	// the actual current ray, using its regular movement restore footprint.
	auto x=laser.ray_start_left.v,y=laser.ray_start_y.v;
	for(int i=0;i<laser.ray_length;++i) {
		egc_copy_rect_1_to_0_16(x>>8,y>>8,16,1);
		x+=laser.step_x.v;y+=laser.step_y.v;
	}
}
namespace th01_native {
void update_shootout_lasers() {
	for(auto& laser:shootout_lasers) laser.update_hittest_and_render();
}
}
