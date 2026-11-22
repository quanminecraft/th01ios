#pragma once
#include "sprites.hpp"

namespace th01_native {
// Presentation storage only: no PC-98 CPU, BIOS, bus, or instruction execution.
// Two pages and four color planes mirror the game's rendering operations.
class Framebuffer {
public:
	static constexpr int width=640, height=400, stride=80, plane_size=stride*height;
	using Plane = std::array<std::uint8_t,plane_size>;
	using Page = std::array<Plane,4>;
	std::array<Page,2> pages{};
	Palette palette{};
	int display_scroll_y=0;
	// Text presentation is separate from the two graphics pages. Zero pixels
	// are transparent; other entries are fixed text colors in ARGB format.
	std::vector<std::uint32_t> text_overlay;
 // A small presentation-space HUD badge, independent of VRAM/palette/scroll.
 struct StatusOverlay {int left=0,top=0,width=0,height=0;std::vector<std::uint32_t> pixels;};
 StatusOverlay status_overlay;
	void clear(unsigned page, std::uint8_t color) {
		if(color > 15) throw std::runtime_error("Invalid color index");
		for(unsigned p = 0; p < 4; ++p) pages.at(page)[p].fill(color&(1u<<p) ? 255 : 0);
	}
	std::uint8_t pixel(unsigned page, int x, int y) const {
		if(x<0 || x>=width || y<0 || y>=height) throw std::runtime_error("Pixel outside framebuffer");
		auto byte = y*stride+x/8;
		auto bit = 0x80u>>(x%8);
		std::uint8_t color=0;
		for(unsigned p=0;p<4;++p) if(pages.at(page)[p][byte]&bit) color |= 1u<<p;
		return color;
	}
	void put(unsigned page, int x, int y, std::uint8_t color) {
		if(x<0 || x>=width || y<0 || y>=height) return;
		if(color>15) throw std::runtime_error("Invalid color index");
		auto byte=y*stride+x/8;
		auto bit=std::uint8_t(0x80u>>(x%8));
		for(unsigned p=0;p<4;++p) {
			auto& dst=pages.at(page)[p][byte];
			dst = (dst&~bit) | (color&(1u<<p) ? bit : 0);
		}
	}
	void blit(unsigned page,int left,int top,const Sprite& sprite,bool alpha=true,std::uint8_t mono=15) {
		if(sprite.color.size()!=std::size_t(sprite.width*sprite.height) || sprite.opaque.size()!=sprite.color.size())
			throw std::runtime_error("Invalid sprite storage");
		// Like ptn_put_8/grc_put_8, align left downward to an 8-pixel boundary.
		left = left>=0 ? left/8*8 : -((-left+7)/8)*8;
		for(int y=std::max(0,-top);y<std::min(sprite.height,height-top);++y)
			for(int x=std::max(0,-left);x<std::min(sprite.width,width-left);++x) {
				auto i=std::size_t(y*sprite.width+x);
				if(!alpha || sprite.opaque[i]) put(page,left+x,top+y,mono<15 ? mono : sprite.color[i]);
			}
	}
	void restore(unsigned from,unsigned to,int left,int top,int w,int h) {
		for(int y=std::max(0,top);y<std::min(height,top+h);++y)
			for(int x=std::max(0,left);x<std::min(width,left+w);++x) put(to,x,y,pixel(from,x,y));
	}
	std::vector<std::uint32_t> rgba(unsigned page) const {
		std::vector<std::uint32_t> out(width*height);
		for(int y=0;y<height;++y) for(int x=0;x<width;++x) out[y*width+x]=palette[pixel(page,x,(y+display_scroll_y)%height)];
  const auto& badge=status_overlay;
  if(!badge.pixels.empty()) {
   if(badge.width<=0||badge.height<=0||badge.pixels.size()!=std::size_t(badge.width*badge.height))throw std::runtime_error("Invalid status overlay");
   for(int y=0;y<badge.height;++y)for(int x=0;x<badge.width;++x) {
    int dx=badge.left+x,dy=badge.top+y;
    if(dx>=0&&dx<width&&dy>=0&&dy<height&&badge.pixels[y*badge.width+x])out[dy*width+dx]=badge.pixels[y*badge.width+x];
   }
  }
		if(!text_overlay.empty()) {
			if(text_overlay.size()!=out.size()) throw std::runtime_error("Invalid text overlay size");
			for(unsigned i=0;i<out.size();++i) if(text_overlay[i]) out[i]=text_overlay[i];
		}
		return out;
	}
};
}
