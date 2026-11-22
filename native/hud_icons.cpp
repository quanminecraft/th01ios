#include "hud_icons.hpp"
namespace {
th01_native::HudIconsBackend* active=nullptr;
th01_native::HudIconsBackend& backend() {
	if(!active) throw std::runtime_error("Native HUD icon backend is not bound");
	return *active;
}
constexpr int LIVES_LEFT=128,LIVES_TOP=0,LIVES_PER_ROW=6,BOMBS_LEFT=128,BOMBS_TOP=16;
constexpr int COL_W=16,ROW_H=16,PTN_LIFE_QUARTER=0,PTN_BOMB_QUARTER=1;
screen_x_t col_left(screen_x_t left,int col) {return left+col*COL_W;}
void snap(screen_x_t left,screen_y_t top,int id,int quarter) {
	auto& b=backend();auto& pixels=b.backgrounds[{id,quarter}];left=left/8*8;
	for(int y=0;y<16;++y) for(int x=0;x<16;++x) pixels[y*16+x]=b.framebuffer.pixel(th01_native::current_render_page(),left+x,top+y);
}
void restore(screen_x_t left,screen_y_t top,int id,int quarter) {
	auto& b=backend();auto it=b.backgrounds.find({id,quarter});
	if(it==b.backgrounds.end()) throw std::runtime_error("HUD icon background was not captured: id="+std::to_string(id)+" quarter="+std::to_string(quarter));
	left=left/8*8;
	for(int y=0;y<16;++y) for(int x=0;x<16;++x) b.framebuffer.put(th01_native::current_render_page(),left+x,top+y,it->second[y*16+x]);
}
void graph_copy_hud_row_0_to_1_8(screen_x_t left,screen_y_t top,pixel_t width) {
	auto& fb=backend().framebuffer;left=left/8*8;
	fb.restore(0,1,left,top,width/16*16,16);page_access(0);
}
}
namespace th01_native {
HudIconsBinding::HudIconsBinding(HudIconsBackend& value):previous(active) {active=&value;}
HudIconsBinding::~HudIconsBinding() {active=previous;}
}
void native_hud_bg_snap(screen_x_t left,screen_y_t top,int id,int quarter) {snap(left,top,id,quarter);}
void native_hud_bg_restore(screen_x_t left,screen_y_t top,int id,int quarter) {restore(left,top,id,quarter);}
void native_hud_copy_row(screen_x_t left,screen_y_t top,pixel_t width) {graph_copy_hud_row_0_to_1_8(left,top,width);}
std::uint32_t& native_hud_hiscore() {return backend().hiscore;}
#define bg_put(first_left,col,top,id,quarter) restore(col_left(first_left,col),top,id,quarter)
#define bg_snap(first_left,col,top,id,quarter) snap(col_left(first_left,col),top,id,quarter)
#define sprite_put(first_left,col,top,id,quarter) ptn_put_quarter_8(col_left(first_left,col),top,id,quarter)
#define ptn_id_and_quarter_from_i(func,first_left,col,top,id,i) func(first_left,col,top,(id+(i/4)),(i%4))
#define rem_lives backend().lives
#define lives_bg_snap_and_put native_hud_lives_init_original
#define bombs_bg_snap_and_put native_hud_bombs_init
#define hud_lives_put native_hud_lives_render_original
#define hud_bombs_put native_hud_bombs_render
#include "th01/main/hud/hud_icons.inc"
#undef hud_lives_put
void native_hud_lives_init() {if(!backend().hide_lives)native_hud_lives_init_original();else if(backend().challenge_init)backend().challenge_init();}
void native_hud_lives_render(int previous) {if(!backend().hide_lives)native_hud_lives_render_original(previous);}
