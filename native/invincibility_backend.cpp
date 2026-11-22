#include "invincibility_backend.hpp"
#include "pellet_sprites.inc"
const std::array<NativeShape8,SHAPE8X8_COUNT> sSHAPE8X8=[]() {
	std::array<NativeShape8,SHAPE8X8_COUNT> shapes{};
	for(unsigned i=0;i<shapes.size();++i) for(unsigned y=0;y<8;++y) shapes[i].dots[y]=native_shape_dots[i][y];
	return shapes;
}();
void shape8x8_invincibility_put(screen_x_t left,vram_y_t top,int cel) {
	if(left<0 || cel>SHAPE8X8_INVINCIBILITY_CELS-1) return;
	if(cel<0) throw std::out_of_range("Negative invincibility cel");
	page_access(0);auto& fb=th01_native::graphics_framebuffer();
	// The upstream first pass redraws a subset of these same red pixels,
	// masked by page 1's B plane. The following full pass overwrites all of
	// them again, so the final image is this mask. No previous cel is erased.
	for(int y=0;y<8;++y) for(int x=0;x<8;++x) if(sSHAPE8X8[SHAPE8X8_INVINCIBILITY+cel][y]&(0x80u>>x)) {
		int linear=(top+y)*640+left+x;
		if(linear>=0 && linear<640*400) fb.put(0,linear%640,linear/640,2);
	}
}
