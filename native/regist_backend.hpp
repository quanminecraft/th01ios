#pragma once
#include "scoredat_backend.hpp"
#include "fullwidth_text.hpp"
#include "input_backend.hpp"
#include "th01/math/clamp.hpp"
#include "bomb_backend.hpp"
using shiftjis_t=char;using vc_t=std::uint8_t;using vram_y_t=std::int16_t;
template<unsigned N> union ShiftJISKanjiBuffer {shiftjis_kanji_t t[N];char byte[N*2];unsigned char ubyte[N*2];};
namespace th01_native {
struct RegistBackend {
	std::function<InputKeys()> poll;
	std::function<void(unsigned)> wait;
	std::function<void()> present;
};
class RegistBinding {
	RegistBackend* previous;
public:
	explicit RegistBinding(RegistBackend& backend);
	~RegistBinding();
	RegistBinding(const RegistBinding&)=delete;
	RegistBinding& operator=(const RegistBinding&)=delete;
};
}
void regist_menu(score_t score,std::int16_t stage,shiftjis_kanji_t route);
void native_regist_input(bool16 reset);void native_regist_reset();void native_regist_wait(unsigned frames);
inline void native_regist_text(screen_x_t left,screen_y_t top,int16_t effect,const char* text) {graph_putsa_fx(left,top,effect,text);}
inline void native_regist_text(screen_x_t left,screen_y_t top,int16_t effect,const unsigned char* text) {graph_putsa_fx(left,top,effect,reinterpret_cast<const char*>(text));}
void graph_2xscale_byterect_1_to_0_slow(screen_x_t left,screen_y_t top,screen_x_t src_left,screen_y_t src_top,pixel_t width,pixel_t height);
void graph_move_byterect_interpage(screen_x_t src_left,screen_y_t src_top,screen_x_t right,screen_y_t bottom,screen_x_t dst_left,screen_y_t dst_top,unsigned source);
constexpr int FX_CLEAR_BG=0x200,FX_REVERSE=0x800;
#define COL_REGIST_SELECTED 3
#define COL_REGIST_REGULAR V_WHITE
