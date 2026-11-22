#pragma once
#include "font_rom.hpp"
#include <string>
namespace th01_native {
// Snapshot/restore the original lives field, just like the score digits.
class ChallengeHud {
 static constexpr int left=128,top=0,width=96,height=16;
 std::array<std::array<std::uint8_t,width*height>,2> background{};
 bool ready=false;
public:
 void reset() {ready=false;}
 void capture(const Framebuffer& fb) {
  for(unsigned page=0;page<2;++page)for(int y=0;y<height;++y)for(int x=0;x<width;++x)
   background[page][y*width+x]=fb.pixel(page,left+x,top+y);
  ready=true;
 }
 void draw(Framebuffer& fb,const FontRom& font,std::int32_t misses) const {
  fb.status_overlay={};if(!ready)return;
  const auto value=std::to_string(misses);
  for(unsigned page=0;page<2;++page) {
   for(int y=0;y<height;++y)for(int x=0;x<width;++x)fb.put(page,left+x,top+y,background[page][y*width+x]);
   font.shift_jis(fb,page,left,top,0x27,"\x81\x87");
   if(value.size()<=4) {
    std::string wide;
    for(char c:value) {
     if(c=='-')wide+="\x81\x7c";
     else {wide+=char(0x82);wide+=char(0x4f+c-'0');}
    }
    font.shift_jis(fb,page,left+24,top,0x27,wide);
   }else if(value.size()<=9)font.ascii(fb,page,left+24,top,0x27,value);
   else {
    // Preserve the complete signed value even beyond ordinary play counts.
    int cell=72/int(value.size());
    for(unsigned i=0;i<value.size();++i) {
     auto glyph=font.jis(0x2900+static_cast<unsigned char>(value[i]));
     for(int y=0;y<16;++y)for(int x=0;x<cell;++x)
      if(glyph.opaque[y*8+x*8/cell])fb.put(page,left+24+i*cell+x,top+y,7);
    }
   }
  }
 }
};
}
