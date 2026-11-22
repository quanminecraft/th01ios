#pragma once
#include "ending_picture.hpp"
#include "ending_palette.hpp"
#include <cstdio>
#include <utility>

namespace th01_native {
struct EndingHost {
 std::function<SpriteSet(const char*)> load;
 std::function<void(unsigned)> wait;
 std::function<void(int,int,int,const char*)> text;
 std::function<void(const char*,const char*)> audio;
 std::function<void()> verdict;
};
class EndingSequence {
 using vc_t=unsigned;
 using screen_x_t=int;
 using vram_y_t=int;
 using sshiftjis_t=char;
 using sshiftjis_kanji_t=char;
 static constexpr int GLYPH_HALF_W=8,GLYPH_FULL_W=16,GLYPH_H=16;
 static constexpr int FX_WEIGHT_NORMAL=0,RES_X=640,BYTE_MASK=7;
 static constexpr int CUTSCENE_PIC_TOP=100,CUTSCENE_PIC_H=200;
 static constexpr int FINAL_DELAY_FRAMES=300,ES_MAKAI=1,ES_JIGOKU=2;
 Framebuffer& framebuffer;
 EndingHost host;
 EndingPalette palette;
 unsigned page=0;
 int continues_total,end_flag;
 bool challenge_bad;
 void frame_delay(unsigned frames) {host.wait(frames);}
 void page_access(unsigned p) {page=p;}
 void z_graph_clear() {framebuffer.clear(page,0);}
 void graph_scrollup(unsigned y) {framebuffer.display_scroll_y=y;}
 void end_pic_show(int quarter) {ending_picture_show(framebuffer,quarter);page=0;}
 void end_pics_load_palette_show(const char* name) {
  page=1;
  auto image=host.load(name);
  if(image.images.size()!=1||image.images[0].width!=640||image.images[0].height!=400)
   throw std::runtime_error("Invalid ending GRP dimensions");
  framebuffer.blit(1,0,0,image.images[0],false);
  palette.load(image.palette);
 }
 void grp_palette_settone(int tone) {palette.grp_palette_settone(tone);}
 void grp_palette_black_out(unsigned frames) {palette.grp_palette_black_out(frames);}
 void grp_palette_black_in(unsigned frames) {palette.grp_palette_black_in(frames);}
 void grp_palette_white_out(unsigned frames) {palette.grp_palette_white_out(frames);}
 void grp_palette_white_in(unsigned frames) {palette.grp_palette_white_in(frames);}
 void mdrv2_bgm_load(const char* name) {host.audio("load",name);}
 void mdrv2_bgm_play() {host.audio("play","");}
 void mdrv2_bgm_stop() {host.audio("stop","");}
 void mdrv2_bgm_fade_out_nonblock() {host.audio("fade","");}
 void verdict_animate_and_regist_menu() {host.verdict();}
 template<class... Args> void graph_printf_fx(int x,int y,int fx,const char* fmt,Args... args) {
  char text[8]{};
  auto size=std::snprintf(text,sizeof(text),fmt,args...);
  if(size<0||size>=int(sizeof(text)))throw std::runtime_error("Ending typed glyph overflow");
  host.text(x,y,fx,text);
 }
#include "ending_captions.inc"
#define pic_caption_type_n original_pic_caption_type_n
#include "th01/end/text_type.inc"
#undef pic_caption_type_n
 void pic_caption_type_n(int line,size_t len,const char* text) {
  if(challenge_bad) {
   const std::string caption=text;
   if(caption==END_BAD_LINE_1)text="Try 10 misses or less!";
   else if(caption==END_BAD_LINE_2_MAKAI||caption==END_BAD_LINE_2_JIGOKU)text="Challenge - Bad Ending";
   len=std::char_traits<char>::length(text);
  }
  original_pic_caption_type_n(line,len,text);
 }
#define near
#define end_pic_show_and_delay(quarter, frames) {end_pic_show(quarter);frame_delay(frames);}
#define end_pic_white_in_and_delay(quarter, speed, frames) {end_pic_show(quarter);grp_palette_white_in(speed);frame_delay(frames);}
#include "th01/end/sequence.inc"
#undef near
#undef end_pic_show_and_delay
#undef end_pic_white_in_and_delay
#undef graph_type_ank
#undef graph_type_kanji
#undef pic_caption_type_2
#undef END_BAD_LINE_1
#undef END_BAD_LINE_2_MAKAI
#undef END_BAD_LINE_2_JIGOKU
#undef END_GOOD_LINE_1
#undef END_GOOD_LINE_2_MAKAI
#undef END_GOOD_LINE_2_JIGOKU
#undef SLIDES_TITLE_5
#undef SLIDES_TITLE_10
#undef SLIDES_TITLE_15
#undef SLIDES_TITLE_20
#undef SLIDES_BOSS_5
#undef SLIDES_BOSS_10_MAKAI
#undef SLIDES_BOSS_10_JIGOKU
#undef SLIDES_BOSS_15_MAKAI
#undef SLIDES_BOSS_15_JIGOKU
#undef SLIDES_BOSS_20_MAKAI
#undef SLIDES_BOSS_20_JIGOKU
public:
 EndingSequence(Framebuffer& fb,EndingHost callbacks,bool makai,unsigned continues,bool challenge=false)
  :framebuffer(fb),host(std::move(callbacks)),palette(fb,host.wait),
   continues_total(continues!=0),end_flag(makai?ES_MAKAI:ES_JIGOKU),challenge_bad(challenge&&continues!=0) {
  if(!host.load||!host.wait||!host.text||!host.audio||!host.verdict)
   throw std::invalid_argument("Ending requires all host callbacks");
 }
 void run() {end_and_verdict_and_regist_animate();}
 EndingPalette& colors() {return palette;}
};
}
