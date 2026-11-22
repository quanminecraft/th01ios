#pragma once
#include "pellet_backend.hpp"
#include "shape_types.hpp"
#include "th01/sprites/main_grc.h"
constexpr int RES_Y=400,COLOR_COUNT=16,COMPONENT_COUNT=3,GRC_IMAGES_PER_SLOT=8,VRAM_PLANE_B=0,VRAM_PLANE_R=1;
using svc_comp_t=std::int8_t;
struct NativeBombColor {
	struct {std::int16_t r=0,g=0,b=0;} c;
	void set(int r,int g,int b) {c.r=r;c.g=g;c.b=b;}
	std::int16_t& component(unsigned i) {if(i==0) return c.r;if(i==1) return c.g;if(i==2) return c.b;throw std::out_of_range("RGB component");}
	const std::int16_t& component(unsigned i) const {return const_cast<NativeBombColor*>(this)->component(i);}
};
struct Palette4 {
	std::array<NativeBombColor,16> colors{};
	Palette4()=default;
	Palette4(std::initializer_list<int> values) {if(values.size()!=48) throw std::invalid_argument("RGB4 palette size");unsigned i=0;for(auto v:values) {colors[i/3].component(i%3)=v;++i;}}
	NativeBombColor& operator[](unsigned i) {return colors.at(i);}
	const NativeBombColor& operator[](unsigned i) const {return colors.at(i);}
};
namespace th01_native {
struct BombBackend {
	Framebuffer& framebuffer;
	const SpriteSet& kuji1;
	const SpriteSet& kuji2;
	Palette4 current,stage;
	std::function<void()> wait_vsync;
	BombBackend(Framebuffer& fb,const SpriteSet& first,const SpriteSet& second,std::function<void()> wait);
};
class BombBackendBinding {
	BombBackend* previous;
	PlayerBackend& player;
	std::function<bool16(int)> previous_update;
public:
	BombBackendBinding(BombBackend& backend,PlayerBackend& player);
	~BombBackendBinding();
	BombBackendBinding(const BombBackendBinding&)=delete;
	BombBackendBinding& operator=(const BombBackendBinding&)=delete;
};
}
bool16 native_bomb_update(int frame);
Palette4& native_bomb_palette();
const Palette4& native_bomb_stage_palette();
void z_palette_set_show(unsigned color,int r,int g,int b);
void z_palette_set_all_show(const Palette4& palette);
void grc_put_8(screen_x_t left,screen_y_t top,main_grc_slot_t slot,int image,unsigned color);
void z_vsync_wait_and_scrollup(screen_y_t top);
void graph_copy_page_to_other(unsigned source);
bool16 graph_invert_32x32_8(screen_x_t left,screen_y_t top,int plane);
#define grc_sloppy_unput(left,top) egc_copy_rect_1_to_0_16(left,top,48,32)
