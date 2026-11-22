#include "fullwidth_text.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main() {
	try {
		Bytes bytes(0x46800);
		for(int digit=0;digit<10;++digit) bytes[0x1800+(0x23-0x21)*96*32+(0x30+digit-0x20)*32]=0x80u>>(digit%8);
		auto glyph_offset=0x1800+(0x23-0x21)*96*32+(0x31-0x20)*32;bytes[glyph_offset+1]=0x14;
		FontRom font(std::move(bytes));auto fb=std::make_unique<Framebuffer>();SpriteSet sprites;
		ShotBackend shots{*fb,sprites,{}};ShotBackendBinding shot_binding(shots);
		CardRenderer renderer{*fb,sprites,font,{}};CardEvents events;events.renderer=&renderer;CardBackendBinding card_binding(events);
		auto clear=[&]() {fb->clear(0,3);page_access(0);};
		clear();graph_putfwnum_fx(0,0,7,7,1234567,0,true);
		for(int i=0;i<7;++i) require(fb->pixel(0,i*16+(i+1)%8,0)==7,"Seven original fullwidth digit positions");
		clear();graph_putfwnum_fx(64,20,7,3,129,123,true);
		require(fb->pixel(0,65,20)==3 && fb->pixel(0,82,20)==3 && fb->pixel(0,97,20)==7,"Only changed digits render for nonzero previous value");
		clear();graph_putfwnum_fx(0,0,7,4,12,0,false);
		require(fb->pixel(0,0,0)==3 && fb->pixel(0,16,0)==3 && fb->pixel(0,33,0)==7 && fb->pixel(0,50,0)==7,"Leading zeros suppressed until first nonzero digit");
		clear();graph_putfwnum_fx(0,0,7,2,123,0,true);require(fb->pixel(0,2,0)==7 && fb->pixel(0,19,0)==7,"Rightmost requested digits retained");
		clear();graph_putfwnum_fx(624,0,7,2,12,0,true);require(fb->pixel(0,625,0)==7,"Last fullwidth cell fits at right screen edge");
		clear();font.shift_jis(*fb,0,10,10,0x27,"\x82\x50");
		require(fb->pixel(0,12,11)==7 && fb->pixel(0,13,11)==3 && fb->pixel(0,14,11)==7 && fb->pixel(0,15,11)==7,"Original bold transformation applies to wide glyph row");
		clear();font.shift_jis(*fb,0,10,10,0x37,"\x82\x50");
		for(int x=11;x<=15;++x) require(fb->pixel(0,x,11)==7,"Original black-weight transformation applies to wide glyph row");
		std::cout << "Original fullwidth number rendering, changed digits, clipping and HUD font weights passed\n";
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}
