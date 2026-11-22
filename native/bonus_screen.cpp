#include "bonus_screen.hpp"
#include "bonus_strings.inc"
#include <stdexcept>
namespace th01_native {
namespace {
int text_width(const std::string& text) {
	int width=0;for(std::size_t i=0;i<text.size();++i) {auto c=static_cast<unsigned char>(text[i]);if(c>=0x80) {++i;width+=16;}else width+=8;}return width;
}
std::string digits(unsigned value,unsigned count) {
	std::string out(count*2,'\0');for(unsigned i=0;i<count;++i) {out[(count-i-1)*2]=char(0x82);out[(count-i-1)*2+1]=char(0x4f+value%10);value/=10;}if(value) throw std::runtime_error("Bonus number exceeds original display width");return out;
}
}
void render_stage_bonus(Framebuffer& fb,const FontRom& font,unsigned stage,const StageBonusValues& values,const std::function<void(unsigned)>& wait) {
	if(!wait) throw std::invalid_argument("Bonus presentation wait is required");
	const int width=std::max(text_width(stagebonus_title),text_width(STAGEBONUS_TOTAL)+16+text_width(stagebonus_digit_buf));
	const int padded_width=width+32,height=40+14*16+16;
	for(int row=0;row<height/2;++row) {
		for(int x=0;x<padded_width;++x) if(native_bonus_box[row%4]&(0x80u>>(x%8))) {
			fb.put(0,32+x,40+height/2-row,0);fb.put(0,32+x,40+height/2+row,0);
		}
		if(row%6==0) wait(1);
	}
	auto put=[&](int left,int row,unsigned color,const std::string& text) {font.shift_jis(fb,0,48+left,80+row*16,0x20|color,text);};
	std::string title=stagebonus_title;title.replace(std::string(STAGEBONUS_TITLE_PREFIX).size(),4,digits(stage,2));
	put(width-text_width(stagebonus_title),0,10,title);
	put(0,2,7,STAGEBONUS_SUBTITLE);put(0,3,7,STAGEBONUS_TIME);put(0,5,7,STAGEBONUS_CARDCOMBO_MAX);put(0,7,7,STAGEBONUS_RESOURCES);put(0,9,7,STAGEBONUS_STAGE_NUMBER);put(0,11,7,STAGEBONUS_TOTAL);
	put(width-32-text_width(STAGEBONUS_HIT_KEY),13,10,STAGEBONUS_HIT_KEY);
	for(unsigned i=0;i<4;++i) {
		// Out-of-range negative metrics would index outside the original numeral
		// table. Reject rather than silently invent a different visual result.
		if(values.metrics[i]<0) throw std::runtime_error("Original bonus numeral lookup outside 0..9");
		put(width+16-96,3+i*2,7,digits(values.metrics[i],4)+digits(0,1)+"\x81\x40");
	}
	put(width-96,11,7,digits(values.total/10,5)+digits(0,1));
}
}
