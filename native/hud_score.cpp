#include "hud_score.hpp"
#include "fullwidth_text.hpp"
bool fwnum_force_rerender=false;
unsigned char hud_cardcombo_max=0;
namespace {
constexpr int SCORE_LEFT=256,MAX_TOP=0,CUR_TOP=16,COL_W=16,ROW_H=16;
constexpr int MAX_FX=0x27,CUR_FX=0x37;
constexpr int CARDCOMBO_LEFT=400,SCORE_AND_CARDCOMBO_W=176;
screen_x_t col_left(screen_x_t left,int col) {return left+col*COL_W;}
}
#define bg_put(first_left,col,top,id,quarter) native_hud_bg_restore(col_left(first_left,col),top,id,quarter)
#define bg_snap(first_left,col,top,id,quarter) native_hud_bg_snap(col_left(first_left,col),top,id,quarter)
#define ptn_id_and_quarter_from_i(func,first_left,col,top,id,i) func(first_left,col,top,(id+(i/4)),(i%4))
#define digit_changed(var,prev,divisor) (((prev/divisor)%10)!=((var/divisor)%10) || fwnum_force_rerender==true)
#define resident_hiscore native_hud_hiscore()
#define graph_copy_hud_row_0_to_1_8 native_hud_copy_row
#define score_and_cardcombo_bg_snap_and_put native_hud_score_init
#define hud_score_and_cardcombo_render native_hud_score_render
#include "th01/main/hud/hud_score.inc"
