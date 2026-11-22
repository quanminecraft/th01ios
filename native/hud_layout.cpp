#include "hud_layout.hpp"
#include "fullwidth_text.hpp"
bool first_stage_in_scene=true;
namespace {
th01_native::HudLayoutBackend* active=nullptr;
constexpr int STAGE_LEFT=608,STAGE_TOP=32,RANK_CENTER_X=600,RANK_TOP=48,MAX_FX=0x27;
screen_x_t col_left(screen_x_t left,int col) {return left+col*16;}
void hud_bg_put() {
	if(!active) throw std::runtime_error("Native HUD layout is not bound");
	active->mask.put(active->framebuffer,th01_native::current_render_page());
}
unsigned text_extent(const char* text) {return std::char_traits<char>::length(text)*8;}
}
namespace th01_native {
HudLayoutBinding::HudLayoutBinding(HudLayoutBackend& value):previous(active) {active=&value;}
HudLayoutBinding::~HudLayoutBinding() {active=previous;}
}
#undef ptn_put_quarter_noalpha_8
#define ptn_put_quarter_noalpha_8 native_hud_bg_restore
#define bg_snap(first_left,col,top,id,quarter) native_hud_bg_snap(col_left(first_left,col),top,id,quarter)
#define graph_copy_hud_row_0_to_1_8 native_hud_copy_row
#define lives_bg_snap_and_put native_hud_lives_init
#define bombs_bg_snap_and_put native_hud_bombs_init
#define score_and_cardcombo_bg_snap_and_put native_hud_score_init
#define hud_bg_snap_and_put native_hud_layout_render
#include "th01/main/hud/hud_layout.inc"
