#include "ending_palette.hpp"
#include <utility>

namespace th01_native {
EndingPalette::EndingPalette(Framebuffer& fb,std::function<void(unsigned)> callback)
 :framebuffer(fb),wait(std::move(callback)) {
 if(!wait)throw std::invalid_argument("Ending palette requires frame wait");
 // Preserve color zero during tone changes, including nonblack source zero.
 for(unsigned i=0;i<16;++i)for(unsigned c=0;c<3;++c)
  z_Palettes[i].v[c]=(fb.palette[i]>>(16-c*8)&255)/17;
}
void EndingPalette::z_palette_set_all_show(const Palette4& colors) {
 for(unsigned i=0;i<16;++i)
  framebuffer.palette[i]=0xff000000u | (colors[i].v[0]*17u<<16)
   | (colors[i].v[1]*17u<<8) | colors[i].v[2]*17u;
}
void EndingPalette::load(const Palette& colors) {
 for(unsigned i=0;i<16;++i)for(unsigned c=0;c<3;++c) {
  auto value=(colors[i]>>(16-c*8))&255;
  if(value%17)throw std::invalid_argument("Ending GRP palette must be RGB4");
  grp_palette[i].v[c]=value/17;
 }
 if(grp_palette_tone!=100)grp_palette_settone(grp_palette_tone);
 else {
  z_Palettes=grp_palette;
  z_palette_set_all_show(grp_palette);
 }
}
}
