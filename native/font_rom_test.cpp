#include "font_rom.hpp"
#include <memory>
#include <iostream>
using namespace th01_native;
static void require(bool value) {if(!value) throw std::runtime_error("Font ROM test failed");}
int main() {
	Bytes bytes(0x46800);
	auto offset=0x1800+(0x29-0x21)*96*32+('0'-0x20)*32;
	bytes[offset]=0x14;bytes[offset+16]=0xff;
	FontRom font(bytes);auto glyph=font.jis(0x2930);
	require(glyph.width==8 && glyph.opaque[3] && glyph.opaque[5] && !glyph.opaque[0]);
	auto fb=std::make_unique<Framebuffer>();fb->clear(0,3);
	font.ascii(*fb,0,10,10,7,"0");
	require(fb->pixel(0,13,10)==7 && fb->pixel(0,15,10)==7 && fb->pixel(0,12,10)==3);
	font.ascii(*fb,0,10,11,7|0x20,"0");
	require(fb->pixel(0,12,11)==7 && fb->pixel(0,13,11)==3 && fb->pixel(0,14,11)==7 && fb->pixel(0,15,11)==7);
	font.ascii(*fb,0,-8,1,7,"0");require(fb->pixel(0,635,0)==7);
	unsigned errors=0;
	try {FontRom bad(Bytes(1));} catch(const std::runtime_error&) {++errors;}
	try {font.jis(0x7d21);} catch(const std::runtime_error&) {++errors;}
	try {font.ascii(*fb,0,0,0,0x100,"0");} catch(const std::runtime_error&) {++errors;}
	require(errors==3);
	Bytes wide_bytes(0x46800);
	auto first=0x1800+(0x23-0x21)*96*32+(0x31-0x20)*32;
	auto second=0x1800+(0x68-0x21)*96*32+(0x5f-0x20)*32;
	wide_bytes[first]=0x80;wide_bytes[second+16]=1;
	FontRom wide_font(wide_bytes);fb->clear(0,3);
	wide_font.shift_jis(*fb,0,10,20,7,"\x82\x50\xe4\xdd");
	require(fb->pixel(0,10,20)==7 && fb->pixel(0,41,20)==7 && fb->pixel(0,26,20)==3);
	try {wide_font.shift_jis(*fb,0,0,0,7,"\x82\x7f");} catch(const std::runtime_error&) {++errors;}
	try {wide_font.shift_jis(*fb,0,0,0,7,"\x82");} catch(const std::runtime_error&) {++errors;}
	require(errors==5);
	fb->clear(0,3);font.ascii(*fb,0,10,10,7|0xa00,"0");
	require(fb->pixel(0,10,10)==7 && fb->pixel(0,13,10)==0 && fb->pixel(0,18,10)==3);
	fb->clear(0,3);wide_font.shift_jis(*fb,0,10,20,7|0xa00,"\x82\x50");
	require(fb->pixel(0,10,20)==0 && fb->pixel(0,11,20)==7 && fb->pixel(0,26,20)==3);
	std::cout << "Native FONT.ROM layout, halfwidth, bold weight, clipping and bounds tests passed\n";
}
