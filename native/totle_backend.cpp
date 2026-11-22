#include "totle_backend.hpp"
using totle_value_t=std::int32_t;
using vram_word_amount_t=std::int16_t;
#ifndef PTN_QUARTER_W
#define PTN_QUARTER_W 16
#endif
std::uint32_t score_bonus=0;
namespace {
th01_native::TotleBackend* active=nullptr;
th01_native::TotleBackend& backend() {if(!active) throw std::runtime_error("TOTLE screen is not bound");return *active;}
void frame_delay(unsigned n) {backend().wait(n);}
void delay(unsigned ms) {backend().wait(0);backend().delay_ms(ms);}
void native_totle_input(bool16 reset) {backend().wait(0);th01_native::set_input_keys(backend().poll());input_sense(reset);}
void native_totle_reset_input() {th01_native::set_input_keys(backend().poll());input_reset_sense();}
void totle_load_and_pagetrans_animate() {auto& b=backend();b.loaded=true;th01_native::totle_transition(b.framebuffer,b.clear_screen,b.wait);}
bool totle_show_automatic_result() {
 auto& show=backend().automatic_result;
 if(!show)return false;
 show();
 return true;
}
void totle_free() {
 auto& b=backend();const auto full=b.framebuffer.palette;
 for(unsigned frame=1;frame<=30;++frame) {
  for(unsigned i=0;i<full.size();++i) {
   std::uint32_t color=0xff000000u;
   for(unsigned shift:{0u,8u,16u})color|=(((full[i]>>shift)&255)*(30-frame)/30)<<shift;
   b.framebuffer.palette[i]=color;
  }
  b.wait(1);
 }
 b.loaded=false;
}
void native_totle_put(screen_x_t left,screen_y_t top,int id) {
	auto& b=backend();if(!b.loaded) throw std::runtime_error("TOTLE numeral bank is not loaded");
	b.framebuffer.blit(th01_native::current_render_page(),left,top,b.numbers.images.at(id-PTN_TOTLE_NUMERAL_32));
}
void native_totle_quarter(screen_x_t left,screen_y_t top,int id,int quarter) {
	auto& b=backend();if(!b.loaded || quarter<0 || quarter>3) throw std::runtime_error("Invalid TOTLE quarter sprite");
	if(b.sloppy) ptn_sloppy_unput_quarter_16(left,top);
	const auto& sprite=b.numbers.images.at(id-PTN_TOTLE_NUMERAL_32);
	left=left/8*8;int sx=(quarter&1)*16,sy=(quarter>>1)*16;
	for(int y=0;y<16;++y) for(int x=0;x<16;++x) {auto i=(sy+y)*32+sx+x;if(sprite.opaque.at(i)) b.framebuffer.put(th01_native::current_render_page(),left+x,top+y,sprite.color.at(i));}
}
}
namespace th01_native {
TotleBinding::TotleBinding(TotleBackend& value):previous(active) {
	if(!value.poll || !value.wait || !value.delay_ms || value.numbers.images.size()<13) throw std::invalid_argument("TOTLE input/timing and numeral sprites are required");
	for(auto& sprite:value.numbers.images) if(sprite.width!=32 || sprite.height!=32) throw std::invalid_argument("TOTLE numerals require 32x32 source sprites");active=&value;
}
TotleBinding::~TotleBinding() {active=previous;}
}
#define rem_lives native_extend_lives()
#define ptn_sloppy_unput_before_alpha_put backend().sloppy
#define ptn_put_8 native_totle_put
#define ptn_put_quarter_8 native_totle_quarter
#define input_sense native_totle_input
#define input_reset_sense native_totle_reset_input
#include "th01/main/totle_coords.inc"
#include "th01/main/bonus_accumulate.inc"
#include "th01/main/totle_digits.inc"
#include "th01/main/totle_animate.inc"
