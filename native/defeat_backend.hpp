#pragma once
#include "singyoku_backend.hpp"
#include "input_backend.hpp"
#include "font_rom.hpp"
#include "shiftjis_width.hpp"
#include "th01/route.hpp"
#include "th01/main/boss/defeat.hpp"
using svc2=std::int16_t;using vc2=std::uint8_t;
struct RGB4 {static constexpr int max() {return 15;}};
namespace th01_native {
struct DefeatBackend {
	Framebuffer& framebuffer;
	const FontRom& font;
	const Palette4& post_palette;
	route_t& route;
	std::function<InputKeys()> poll;
 bool skip_route_selection=false;
};
class DefeatBinding {
	DefeatBackend* previous;
public:
	explicit DefeatBinding(DefeatBackend& backend);
	~DefeatBinding();
	DefeatBinding(const DefeatBinding&)=delete;
	DefeatBinding& operator=(const DefeatBinding&)=delete;
};
}
const Palette4& native_defeat_palette();
void native_defeat_route_set(route_t route);
bool native_defeat_skip_route();
void native_defeat_input(bool16 reset);
void native_defeat_reset_input();
void native_defeat_whiteline(screen_y_t y);
void z_graph_clear();
void player_put_default();
void graph_glyphrow_put(int row,int color,const char* text);
void graph_glyphrow_2xscale_1_to_0(screen_x_t left,vram_y_t top,int row,pixel_t width);
#define boss_post_defeat_palette native_defeat_palette()
#define defeat_comp(color,i) (color).component(i)
#define palette_foreach(col,comp,func) for(col=0;col<16;++col) {for(comp=0;comp<3;++comp) {func}}
#define z_Palettes_set_func_and_show(col,comp,func) {palette_foreach(col,comp,func);z_palette_set_all_show(z_Palettes);}
#define z_palette_white_out_step(col,comp) z_Palettes_set_func_and_show(col,comp,{if(defeat_comp(z_Palettes[col],comp)<15) ++defeat_comp(z_Palettes[col],comp);})
#define z_palette_set_white(col,comp) z_Palettes_set_func_and_show(col,comp,{defeat_comp(z_Palettes[col],comp)=15;})
