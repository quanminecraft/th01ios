#include "item_motion.hpp"
#include "th01/math/polar.hpp"
namespace {
constexpr pixel_t ITEM_W=32,ITEM_H=32;
int floor8(int x) {return x>=0 ? x/8 : -((-x+7)/8);}
}
namespace th01_native {
// Native rendering boundary for shape.cpp's cached ellipse dots. Preserve
// its omitted last cache flush and its test of the next point's X coordinate.
void item_splash_put(screen_x_t cx,screen_y_t cy,pixel_t radius,unsigned color,unsigned char step) {
	if(!step) throw std::invalid_argument("Zero splash angle step");
	auto& fb=graphics_framebuffer();int cached=-1;unsigned dots=0;
	for(int angle=0;angle<=255;angle+=step) {
		int x=polar_x(cx,radius,angle),y=polar_y(cy,radius,angle);
		int offset=static_cast<std::int16_t>(y*80+floor8(x));
		if(cached!=offset) {
			if(x>=0 && x<640 && cached>=0 && cached<32000)
				for(int bit=0;bit<8;++bit) if(dots&(0x80u>>bit)) fb.put(current_render_page(),(cached%80)*8+bit,cached/80,color);
			dots=0x80u>>(x&7);cached=offset;
		} else dots|=0x80u>>(x&7);
	}
}
void item_splash_unput(screen_x_t cx,screen_y_t cy,pixel_t radius,unsigned char step) {
	if(!step) throw std::invalid_argument("Zero splash angle step");
	// DOS reads uninitialized previous coordinates on the first point.
	// Native explicitly restores that point, then preserves the original
	// duplicate suppression and the oversized 16-pixel EGC restore boundary.
	bool first=true;int prev_x=0,prev_y=0;
	for(int angle=0;angle<=255;angle+=step) {
		int x=polar_x(cx,radius,angle),y=polar_y(cy,radius,angle);
		if(first || prev_y!=y || floor8(prev_x)!=floor8(x)) {
			egc_copy_rect_1_to_0_16(x,y,8,1);prev_x=x;prev_y=y;first=false;
		}
	}
}
}
#define shape_circle_put th01_native::item_splash_put
#define shape_circle_sloppy_unput th01_native::item_splash_unput
#include "th01/main/stage/item_motion.inc"
#undef shape_circle_put
#undef shape_circle_sloppy_unput
