#pragma once
#include "archive.hpp"
#include <array>
#include <algorithm>

namespace th01_native {
using Palette = std::array<std::uint32_t,16>;
struct Sprite {
	int width = 0, height = 0;
	Bytes color, opaque;
};
struct SpriteSet {
	Palette palette{};
	std::vector<Sprite> images;
	bool monochrome = false;
};
inline SpriteSet read_sprites(const Bytes& file) {
	if(file.size() < 64) throw std::runtime_error("Truncated sprite header");
	const std::string magic(file.begin(),file.begin()+4);
	bool ptn = magic == "HPTN", grc = magic == "GRCG", bos = magic == "BOSS";
	if(!ptn && !grc && !bos) throw std::runtime_error("Unknown sprite format");
	int width = ptn ? 32 : (grc ? (file[4]+256*file[5])*8 : file[4]*8);
	int height = ptn ? 32 : (grc ? file[6]+256*file[7] : file[6]);
	unsigned count = file[ptn ? 5 : 8];
	if(width <= 0 || width > 640 || height <= 0 || height > 400 || !count || count > 64)
		throw std::runtime_error("Invalid sprite dimensions/count");
	std::size_t palette_start = ptn ? 6 : 16;
	std::size_t plane_bytes = (width/8)*height;
	std::size_t image_bytes = ptn ? 513 : plane_bytes*(grc ? 1 : 5);
	std::size_t start = palette_start+48;
	if(file.size() != start+count*image_bytes) throw std::runtime_error("Sprite file size mismatch");
	SpriteSet result;
	result.monochrome = grc;
	for(unsigned c = 0; c < 16; ++c) {
		auto p = palette_start+c*3;
		if(file[p] > 15 || file[p+1] > 15 || file[p+2] > 15)
			throw std::runtime_error("Invalid RGB4 palette");
		result.palette[c] = 0xff000000u | (file[p]*17u<<16) | (file[p+1]*17u<<8) | file[p+2]*17u;
	}
	for(unsigned i = 0; i < count; ++i) {
		auto base = start+i*image_bytes+(ptn ? 1 : 0);
		Sprite sprite{width,height,Bytes(width*height),Bytes(width*height)};
		for(int y = 0; y < height; ++y) for(int x = 0; x < width; ++x) {
			auto byte = std::size_t(y*(width/8)+x/8);
			auto bit = 0x80u >> (x%8);
			auto pixel = std::size_t(y*width+x);
			if(grc) {
				sprite.color[pixel] = 15;
				sprite.opaque[pixel] = (file[base+byte]&bit) != 0;
			} else {
				// PC-98 plane order B,R,G,E corresponds to index bits 0,1,2,3.
				auto color_start = base+(bos ? plane_bytes : 0);
				for(unsigned plane = 0; plane < 4; ++plane)
					if(file[color_start+plane*plane_bytes+byte]&bit) sprite.color[pixel] |= 1u<<plane;
				// BOS stores a transparency mask: 1 preserves the background.
				sprite.opaque[pixel] = bos ? ((file[base+byte]&bit) == 0) : sprite.color[pixel] != 15;
			}
		}
		result.images.push_back(std::move(sprite));
	}
	return result;
}
}
