#pragma once
#include "sprites.hpp"

namespace th01_native {
inline Sprite read_hud_mask(const Bytes& file) {
	if(file.size()<64 || std::string(file.begin(),file.begin()+4)!="HGRF")
		throw std::runtime_error("Invalid HUD mask");
	std::size_t size=file[8]+256*file[9],pos=64;
	if(size>file.size()-pos) throw std::runtime_error("Truncated HUD stream");
	auto end=pos+size;
	Bytes dots;
	// HUD code reads only the first plane and draws the first 66 lines in
	// palette index V_RED (10), retaining the stage's current palette.
	while(pos<end && dots.size()<66*80) {
		auto value=file[pos++];
		unsigned count=1;
		if(value==file[5]) {
			if(pos==end) throw std::runtime_error("Truncated HUD run");
			auto extra=file[pos++];
			if(extra) {
				if(pos==end) throw std::runtime_error("Truncated HUD run value");
				value=file[pos++];count+=extra;
			}
		}
		dots.insert(dots.end(),count,value);
	}
	if(dots.size()<66*80) throw std::runtime_error("Short HUD mask");
	Sprite sprite{640,66,Bytes(640*66,10),Bytes(640*66)};
	for(int y=0;y<66;++y) for(int x=0;x<640;++x)
		sprite.opaque[y*640+x]=(dots[y*80+x/8]&(0x80u>>(x%8)))!=0;
	return sprite;
}
}
