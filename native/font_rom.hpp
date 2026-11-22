#pragma once
#include "framebuffer.hpp"

namespace th01_native {
// V98 0x46800-byte FONT.ROM file layout, independently read as bitmap data.
// Layout reference: AZO234/NP2kai font/fontv98.c (no emulator code is linked).
class FontRom {
	Bytes bytes;
public:
	explicit FontRom(Bytes data):bytes(std::move(data)) {
		if(bytes.size()!=0x46800) throw std::runtime_error("Unsupported FONT.ROM size");
	}
	Sprite ank16(unsigned codepoint) const {
		if(codepoint>255) throw std::runtime_error("ANK glyph outside FONT.ROM");
		Sprite glyph{8,16,Bytes(128,7),Bytes(128)};
		for(int y=0;y<16;++y) for(int x=0;x<8;++x)
			glyph.opaque[y*8+x]=(bytes.at(0x800+codepoint*16+y)&(0x80u>>x))!=0;
		return glyph;
	}
	Sprite jis(unsigned codepoint) const {
		unsigned row=codepoint>>8,col=codepoint&255;
		if(row<0x21 || row>0x7c || col<0x20 || col>0x7f) throw std::runtime_error("JIS glyph outside FONT.ROM");
		auto offset=0x1800+(row-0x21)*96*32+(col-0x20)*32;
		int width=(row==0x29 || row==0x2a) ? 8 : 16;
		Sprite glyph{width,16,Bytes(width*16,7),Bytes(width*16)};
		for(int y=0;y<16;++y) for(int x=0;x<width;++x)
			glyph.opaque[y*width+x]=(bytes.at(offset+y+(x>=8 ? 16 : 0))&(0x80u>>(x%8)))!=0;
		return glyph;
	}
	void ascii(Framebuffer& fb,unsigned page,int left,int top,unsigned effect,const std::string& text) const {
		if(effect&~0xa3fu) throw std::runtime_error("Text effect is not implemented");
		if(effect&0x200) for(int y=0;y<16;++y) for(unsigned x=0;x<text.size()*8;++x) fb.put(page,left+x,top+y,0);
		unsigned weight=(effect>>4)&3,color=effect&15;
		for(unsigned char c:text) {
			if(c<0x20 || c>0x7e) throw std::runtime_error("Non-ASCII text requires Shift-JIS decoding");
			auto glyph=jis(c==0x20 ? 0x2b21 : 0x2900+c);
			if(left+8>640) break;
			for(int y=0;y<16;++y) {
				std::uint16_t row=0;
				for(int x=0;x<8;++x) if(glyph.opaque[y*glyph.width+x]) row|=0x8000u>>x;
				if(weight==1 || weight==3) row|=row<<1;
				if(weight==2 || weight==3) {
					auto tmp=row;row|=row<<1;tmp^=row;row&=~(tmp<<1);
				}
				if(effect&0x800) row^=0xff00;
				for(int x=0;x<16;++x) if(row&(0x8000u>>x)) {
					int linear=(top+y)*640+left+x;
					if(linear>=0 && linear<640*400) fb.put(page,linear%640,linear/640,color);
				}
			}
			left+=8;
		}
	}
	void shift_jis(Framebuffer& fb,unsigned page,int left,int top,unsigned effect,const std::string& text) const {
		if(effect&~0xa3fu) throw std::runtime_error("Text effect is not implemented");
		if(effect&0x200) for(int y=0;y<16;++y) for(unsigned x=0;x<text.size()*8;++x) fb.put(page,left+x,top+y,0);
		effect&=~0x200u;
		for(std::size_t i=0;i<text.size();++i) {
			unsigned lead=static_cast<unsigned char>(text[i]);
			if(lead>=0x20 && lead<=0x7e) {
				ascii(fb,page,left,top,effect,text.substr(i,1));left+=8;continue;
			}
			if(!((lead>=0x81 && lead<=0x9f) || (lead>=0xe0 && lead<=0xef)) || i+1==text.size())
				throw std::runtime_error("Unsupported or incomplete Shift-JIS character");
			unsigned trail=static_cast<unsigned char>(text[++i]);
			if(trail<0x40 || trail>0xfc || trail==0x7f) throw std::runtime_error("Invalid Shift-JIS trail byte");
			unsigned row=((lead<(0xa0) ? lead-0x81 : lead-0xc1)*2)+0x21;
			unsigned col;
			if(trail>=0x9f) {++row;col=trail-0x7e;}
			else col=trail-(trail<0x7f ? 0x1f : 0x20);
			if(effect&~0x83fu) throw std::runtime_error("Wide text effect is not implemented");
			auto glyph=jis((row<<8)|col);
			if(left+16>640) break;
			unsigned weight=(effect>>4)&3;
			for(int y=0;y<16;++y) {
				std::uint16_t dots=0;
				for(int x=0;x<glyph.width;++x) if(glyph.opaque[y*glyph.width+x]) dots|=0x8000u>>x;
				if(weight==1 || weight==3) dots|=dots<<1;
				if(weight==2 || weight==3) {auto tmp=dots;dots|=dots<<1;tmp^=dots;dots&=~(tmp<<1);}
				if(effect&0x800) dots^=0xffff;
				for(int x=0;x<16;++x) if(dots&(0x8000u>>x)) {
					int linear=(top+y)*640+left+x;
					if(linear>=0 && linear<640*400) fb.put(page,linear%640,linear/640,effect&15);
				}
			}
			left+=16;
		}
	}
};
}
