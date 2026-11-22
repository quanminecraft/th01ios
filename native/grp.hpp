#pragma once
#include "sprites.hpp"

namespace th01_native {
// Native translation of the Pi predictor/MTF stream in libs/piloadc/piloadm.asm.
// Only image compression is decoded; no original machine instructions run.
inline SpriteSet read_grp(const Bytes& file) {
	std::size_t pos=0;
	auto byte=[&]() { if(pos>=file.size()) throw std::runtime_error("Truncated GRP");return file[pos++]; };
	auto word=[&]() { unsigned v=byte();return (v<<8)|byte(); };
	if(file.size()<2 || !((file[0]=='Z' && file[1]=='N') || (file[0]=='P' && file[1]=='i')))
		throw std::runtime_error("Invalid GRP signature");
	bool grp=file[0]=='Z';pos=2;
	while(byte()) {} // NUL-terminated comment
	auto flags=byte();word();
	if(byte()!=4) throw std::runtime_error("Only four-plane Pi images supported");
	for(unsigned i=0;i<4;++i) byte(); // machine identifier
	auto extra=word();
	if(extra>file.size()-pos) throw std::runtime_error("Truncated GRP metadata");
	pos+=extra;
	int width=word(),height=word();
	if(width<=0 || width>1280 || width%8 || height<=0 || height>1024)
		throw std::runtime_error("Invalid GRP dimensions");
	if(flags&0x80) throw std::runtime_error("Pi images without an explicit palette are unsupported");
	SpriteSet result;
	for(unsigned i=0;i<16;++i) {
		unsigned r=byte(),g=byte(),b=byte();
		if(grp) {
			if(r>15 || g>15 || b>15) throw std::runtime_error("Invalid GRP RGB4 palette");
			r*=17;g*=17;b*=17;
		}
		result.palette[i]=0xff000000u|(r<<16)|(g<<8)|b;
	}
	unsigned remaining=0,current=0;
	auto bit=[&]() -> unsigned {
		if(!remaining) {current=byte();remaining=8;}
		return (current>>(--remaining))&1u;
	};
	std::array<std::array<std::uint8_t,16>,16> table{};
	for(unsigned c=0;c<16;++c) for(unsigned i=0;i<16;++i) table[c][i]=(c+16-i)%16;
	auto color=[&](unsigned context) {
		unsigned index;
		if(bit()) index=bit();
		else {
			index=1;
			if(bit()) {
				if(bit()) index=(index<<1)|bit();
				index=(index<<1)|bit();
			}
			index=(index<<1)|bit();
		}
		auto& row=table.at(context);
		auto value=row.at(index);
		for(unsigned i=index;i>0;--i) row[i]=row[i-1];
		row[0]=value;
		return value;
	};
	auto first=color(0),second=color(first);
	Bytes pixels(std::size_t(width)*2);
	for(std::size_t i=0;i<pixels.size();i+=2) {pixels[i]=first;pixels[i+1]=second;}
	const auto end=std::size_t(width)*(height+2);
	pixels.reserve(end);
	int previous_offset=0;
	while(pixels.size()<end) {
		int offset;
		if(bit()) {
			if(!bit()) offset=-2*width;
			else offset=-width+(bit() ? -1 : 1);
		} else if(bit()) offset=-width;
		else offset=pixels[pixels.size()-2]==pixels.back() ? -2 : -4;
		// Both horizontal predictors use the same state, even when the last
		// pair changes from alternating colors to one repeated color.
		int selector=(offset==-2 || offset==-4) ? -4 : offset;
		if(selector==previous_offset) {
			do {
				auto a=color(pixels.back()),b=color(a);
				pixels.push_back(a);pixels.push_back(b);
				if(pixels.size()>=end) break;
			} while(bit());
			previous_offset=0;
		} else {
			previous_offset=selector;
			std::size_t pairs=1;
			if(bit()) {
				unsigned count=1;
				while(bit()) if(++count>20) throw std::runtime_error("Invalid GRP run length");
				for(unsigned i=0;i<count;++i) pairs=(pairs<<1)|bit();
			}
			if(std::size_t(-offset)>pixels.size()) throw std::runtime_error("Invalid GRP predictor");
			auto source=pixels.size()+offset;
			auto length=std::min(pairs*2,end-pixels.size());
			for(std::size_t i=0;i<length;++i) pixels.push_back(pixels.at(source+i));
		}
	}
	Sprite image{width,height,Bytes(pixels.begin()+width*2,pixels.end()),Bytes(std::size_t(width)*height,1)};
	result.images.push_back(std::move(image));
	return result;
}
}
