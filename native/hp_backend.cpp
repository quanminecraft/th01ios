#include "hp_backend.hpp"
unsigned int hud_hp_first_white=0,hud_hp_first_redwhite=0;
void native_hp_point_put(int point) {
	if(point<0 || point>=64) throw std::out_of_range("Boss HP point outside framebuffer");
	auto& fb=th01_native::graphics_framebuffer();
	for(int y=0;y<HP_H;++y) {
		page_access(1);
		for(unsigned page:{1u,0u}) {
			page_access(page);
			for(int x=0;x<7;++x) {
				unsigned color=unsigned(point)<hud_hp_first_redwhite ? 10 : unsigned(point)<hud_hp_first_white ? (x%2==0 ? 10 : 7) : 7;
				fb.put(page,128+point*8+x,48+y,color);
			}
		}
	}
}
