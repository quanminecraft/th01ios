#include "ending_palette.hpp"
#include <iostream>
#include <memory>

int main() {try {
 using namespace th01_native;
 auto fb=std::make_unique<Framebuffer>();
 Palette original{};
 for(unsigned i=0;i<16;++i)
  original[i]=0xff000000u|(i*17u<<16)|((15-i)*17u<<8)|((i*7%16)*17u);
 unsigned calls=0,total=0;
 EndingPalette palette(*fb,[&](unsigned frames) {++calls;total+=frames;});
 palette.load(original);
 for(int requested=-5;requested<=205;++requested) {
  palette.grp_palette_settone(requested);
  int tone=std::max(0,std::min(200,requested));
  if(palette.tone()!=tone||fb->palette[0]!=original[0])throw std::runtime_error("Tone clamp or color zero mismatch");
  for(unsigned i=1;i<16;++i)for(unsigned c=0;c<3;++c) {
   int base=(original[i]>>(16-c*8)&255)/17;
   int expected=tone>100 ? base+(15-base)*(tone-100)/100 : base-base*(100-tone)/100;
   if(int(fb->palette[i]>>(16-c*8)&255)!=expected*17)throw std::runtime_error("Original integer tone rounding mismatch");
  }
 }
 auto check=[&](int tone,unsigned delay) {
  if(calls!=20||total!=20*delay||palette.tone()!=tone)throw std::runtime_error("Original fade timing mismatch");
  calls=total=0;
 };
 palette.grp_palette_black_out(10);check(0,10);
 palette.grp_palette_black_in(6);check(100,6);
 palette.grp_palette_white_out(5);check(200,5);
 palette.grp_palette_white_in(4);check(100,4);
 palette.grp_palette_settone(50);
 auto replacement=original;replacement[1]=0xffffffffu;
 palette.load(replacement);
 if(palette.tone()!=50||fb->palette[1]!=0xff888888u)throw std::runtime_error("Loading new GRP lost tone or integer rounding");
 std::cout<<"Original ending palette: all RGB4 components, integer rounding, clamping, four fade timings and persistent tone passed\n";
}catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}}
