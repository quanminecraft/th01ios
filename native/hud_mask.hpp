#pragma once
#include "framebuffer.hpp"
namespace th01_native {
struct HudMask {
	Bytes dots;
	std::size_t consumed=0;
	void put(Framebuffer& fb,unsigned page) const {
		if(dots.size()<66*80) throw std::runtime_error("HUD mask is not loaded");
		for(unsigned i=0;i<dots.size();++i) for(unsigned bit=0;bit<8;++bit)
			if(dots[i]&(0x80u>>bit)) fb.put(page,(i%80)*8+bit,i/80,2);
	}
};
inline HudMask read_hud_mask(const Bytes& data) {
	if(data.size()<64 || std::string(data.begin(),data.begin()+4)!="HGRF") throw std::runtime_error("Invalid HUD GRF header");
	unsigned size=data[8]+(unsigned(data[9])<<8);
	if(!size || 64+size>data.size()) throw std::runtime_error("Truncated HUD GRF plane");
	HudMask out;std::size_t pos=64;
	auto next=[&]() {if(pos>=64+size) throw std::runtime_error("Truncated HUD RLE command");return data[pos++];};
	while(out.dots.size()<66*80) {
		auto value=next();unsigned runs=0;
		if(value==data[5]) {runs=next();if(runs) value=next();}
		// hud_bg_put() writes the runs, then the same byte once more. Preserve
		// its final-run overshoot beyond the nominal 66-row loop boundary.
		out.dots.insert(out.dots.end(),runs+1,value);
	}
	out.consumed=pos-64;return out;
}
}
