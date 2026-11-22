#include "ending_verdict.hpp"
#include <cstdio>
#include <cstring>
#include <type_traits>
#include <utility>

namespace th01_native {
namespace verdict_literals {
using pixel_t=int;
using shiftjis_t=char;
template<std::size_t N> constexpr int shiftjis_w(const char (&)[N]) {return (N-1)*8;}
#include "ending_verdict_strings.inc"
}
using namespace verdict_literals;
struct EndingVerdict::Impl {
 using screen_x_t=int;
 using screen_y_t=int;
 using vram_y_t=int;
 using vc_t=unsigned;
 using sshiftjis_t=char;
 using sshiftjis_kanji_t=char;
 using score_t=std::int32_t;
 static constexpr int FX_WEIGHT_NORMAL=0,FX_WEIGHT_HEAVY=16;
 static constexpr int GLYPH_HALF_W=8,GLYPH_FULL_W=16,GLYPH_H=16;
 static constexpr int RES_X=640,BYTE_MASK=7,CUTSCENE_PIC_TOP=100,CUTSCENE_PIC_H=200;
 static constexpr int RANK_EASY=0,RANK_NORMAL=1,RANK_HARD=2,RANK_LUNATIC=3,RANK_COUNT=4;
 static constexpr int ES_MAKAI=1,ES_JIGOKU=2,GPF_PALETTE_SHOW=1;
 static constexpr int SCOREDAT_CLEARED_MAKAI=50,SCOREDAT_CLEARED_JIGOKU=60;
 // Original little-endian Shift-JIS codepoint, from scoredat.hpp.
 static constexpr std::uint16_t SCOREDAT_ROUTE_CLEAR=44682;
 Framebuffer& framebuffer;
 EndingPalette& palette;
 VerdictHost host;
 std::uint32_t score,score_highest,continues_total;
 std::array<std::uint32_t,4> continues_per_scene;
 unsigned rank,credit_lives_extra,page=0;
 int end_flag;
 bool input_ok=false,input_shot=false;
 Impl(Framebuffer& fb,EndingPalette& pal,const EndingResult& result,VerdictHost callbacks)
  :framebuffer(fb),palette(pal),host(std::move(callbacks)),score(result.score),
   score_highest(result.highest),continues_total(result.continues),continues_per_scene(result.scene_continues),
   rank(result.rank),credit_lives_extra(result.extra_lives),end_flag(result.makai?ES_MAKAI:ES_JIGOKU) {
  if(rank>=4||credit_lives_extra>4||!host.load||!host.wait||!host.text||!host.input||!host.random||
    !host.registration_colors||!host.registration||!host.resident_clear)
   throw std::invalid_argument("Ending verdict requires valid settings and all callbacks");
 }
 void frame_delay(unsigned count) {host.wait(count);}
 void page_access(unsigned value) {page=value;}
 void graph_copy_page_to_other(unsigned source) {framebuffer.pages[!source]=framebuffer.pages[source];}
 void grp_put(const char* name,int) {
  auto image=host.load(name);
  if(image.images.size()!=1||image.images[0].width!=640||image.images[0].height!=400)
   throw std::runtime_error("Invalid verdict GRP dimensions");
  framebuffer.blit(page,0,0,image.images[0],false);palette.load(image.palette);
 }
 void grp_palette_black_out(unsigned frames) {palette.grp_palette_black_out(frames);}
 void grp_palette_black_in(unsigned frames) {palette.grp_palette_black_in(frames);}
 void grp_palette_settone(int tone) {palette.grp_palette_settone(tone);}
 void sense(bool reset) {auto flags=host.input(reset);input_ok=flags[0];input_shot=flags[1];}
 void input_reset_sense() {sense(true);}
 void input_sense(bool reset) {sense(reset);}
 unsigned irand() {return host.random();}
 void regist_colors_set() {host.registration_colors();}
 void regist_menu(std::uint32_t value,int stage,std::uint16_t route) {host.registration(value,stage,route);}
 bool end_resident_clear() {host.resident_clear();return true;}
 template<class... Args> void print(int x,int y,int fx,const char* fmt,Args... args) {
  char buffer[1024]{};
  int size=std::snprintf(buffer,sizeof(buffer),fmt,args...);
  if(size<0||size>=int(sizeof(buffer)))throw std::runtime_error("Verdict text overflow");
  host.text(x,y,fx,buffer);
 }
 void graph_printf_fx(int x,int y,int fx,const char* text) {host.text(x,y,fx,text);}
 template<class T> void graph_printf_fx(int x,int y,int fx,const char* fmt,T value) {
  if constexpr(std::is_integral_v<T>) {
   // DOS long is 32 bits; native LP64 printf requires an actual unsigned long.
   if(std::strstr(fmt,"lu"))print(x,y,fx,fmt,static_cast<unsigned long>(static_cast<std::uint32_t>(value)));
   else print(x,y,fx,fmt,static_cast<int>(value));
  }else print(x,y,fx,fmt,value);
 }
 void graph_printf_fx(int x,int y,int fx,const char* fmt,int first,int second) {print(x,y,fx,fmt,first,second);}
#include "th01/end/text_type.inc"
#define RANKS_CAPS_CENTERED {" EASY ","NORMAL"," HARD ","LUNATIC"}
#include "th01/end/verdict.inc"
#undef RANKS_CAPS_CENTERED
#undef verdict_line_render0
#undef verdict_line_render1
#undef graph_type_ank
#undef graph_type_kanji
#undef pic_caption_type_2
};
EndingVerdict::EndingVerdict(Framebuffer& fb,EndingPalette& palette,const EndingResult& result,VerdictHost host)
 :impl(std::make_unique<Impl>(fb,palette,result,std::move(host))) {}
EndingVerdict::~EndingVerdict()=default;
void EndingVerdict::run() {impl->verdict_animate_and_regist_menu();}
}
