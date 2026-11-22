#include "ending_picture.hpp"

namespace th01_native {
namespace {
// The original loop operates on planar words. This is plain native storage;
// no device registers or CPU instructions are interpreted.
using upixel_t=unsigned;
using uvram_offset_t=unsigned;
using vram_word_amount_t=int;
using pixel_t=int;
template<class T> struct LTWH { T left,top,width,height; };
constexpr LTWH<upixel_t> CUTSCENE_QUARTERS[]={{0,0,320,200},{320,0,320,200},{0,200,320,200},{320,200,320,200}};
constexpr int PIC_LEFT=160,PIC_TOP=100,PIC_H=200,PIC_VRAM_W=40;
constexpr int EGC_REGISTER_SIZE=2,ROW_SIZE=80;
constexpr unsigned vram_offset_shift(unsigned x,unsigned y) {return y*80+x/8;}
struct PictureCopy {
 Framebuffer& framebuffer;
 unsigned page=0;
 struct egc_temp_t {std::array<std::array<std::uint8_t,2>,4> planes;};
 struct Chunk {
  PictureCopy& copy; unsigned offset;
  operator egc_temp_t() const {
   egc_temp_t value{};
   for(unsigned p=0;p<4;++p)for(unsigned b=0;b<2;++b)
    value.planes[p][b]=copy.framebuffer.pages[copy.page][p].at(offset+b);
   return value;
  }
  void operator=(const egc_temp_t& value) {
   for(unsigned p=0;p<4;++p)for(unsigned b=0;b<2;++b)
    copy.framebuffer.pages[copy.page][p].at(offset+b)=value.planes[p][b];
  }
 };
 Chunk egc_chunk(unsigned offset) {return {*this,offset};}
 void egc_start_copy() {}
 void egc_off() {}
 void page_access(unsigned value) {page=value;}
#define near
#include "th01/end/pic_show.inc"
#undef near
};
}
void ending_picture_show(Framebuffer& framebuffer,unsigned quarter) {
 if(quarter>=4)throw std::out_of_range("Ending picture quarter");
 PictureCopy{framebuffer}.end_pic_show(static_cast<int>(quarter));
}
}
