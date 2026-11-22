#include "text_animation.hpp"
#include <utility>
namespace th01_native {
void scene_title_text_animate(Framebuffer& fb,const FontRom& font,const std::function<void(unsigned)>& wait) {
 if(!wait)throw std::invalid_argument("Scene entrance timing is not bound");
 #include "entrance_jis.inc"
 try {
  for(unsigned jis:entrance_jis) {
   mdrv2_se_play(14);
   auto glyph=font.jis(jis);
   // Original reversed black TRAM spaces hide the graphics. The glyph
   // dots use normal black spaces and reveal the stage beneath them.
   fb.text_overlay.assign(640*400,0xff000000u);
   for(int y=0;y<16;++y)for(int x=0;x<16;++x)if(glyph.opaque[y*16+x])
    for(int yy=0;yy<16;++yy)for(int xx=0;xx<16;++xx)
     fb.text_overlay[(80+y*16+yy)*640+192+x*16+xx]=0;
   wait(8);
  }
  fb.text_overlay.clear();
 }catch(...) {fb.text_overlay.clear();throw;}
}
void stage_number_text_animate(Framebuffer& fb,const FontRom& font,unsigned stage,const std::function<void(unsigned)>& wait) {
	if(!wait || stage>99) throw std::invalid_argument("Invalid stage title presentation");
	auto put=[&](const std::string& text,int left) {
		fb.text_overlay.assign(640*400,0);
		for(unsigned i=0;i<text.size();++i) {
			auto glyph=font.ank16(static_cast<unsigned char>(text[i]));
			for(int y=0;y<16;++y) for(int x=0;x<8;++x) if(glyph.opaque[y*8+x])
				for(int yy=0;yy<16;++yy) for(int xx=0;xx<16;++xx)
					fb.text_overlay[(80+y*16+yy)*640+left+int(i)*128+x*16+xx]=0xffff0000u;
		}
	};
	try {put("STAGE",0);wait(35);std::string number{char('0'+stage/10),char('0'+stage%10)};put(number,192);wait(35);fb.text_overlay.clear();}
	catch(...) {fb.text_overlay.clear();throw;}
}
void harryup_text_animate(Framebuffer& fb,const FontRom& font,const std::function<void(unsigned)>& wait) {
	if(!wait) throw std::invalid_argument("Text animation timing/presentation is not bound");
	auto put=[&](const char* text,int left) {
		fb.text_overlay.assign(640*400,0);
		for(unsigned i=0;text[i];++i) {
			auto glyph=font.ank16(static_cast<unsigned char>(text[i]));
			for(int y=0;y<16;++y) for(int x=0;x<8;++x) if(glyph.opaque[y*8+x])
				for(int yy=0;yy<16;++yy) for(int xx=0;xx<16;++xx)
					fb.text_overlay[(80+y*16+yy)*640+left+int(i)*128+x*16+xx]=0xffff0000u;
		}
	};
	try {
		// Five leading TRAM rows = 80 pixels. Five 8-dot letters fill a row;
		// the second word has twelve fullwidth cells of margin on either side.
		put("HARRY",0);wait(27);
		put("UP",192);wait(27);
		fb.text_overlay.clear();
	} catch(...) {fb.text_overlay.clear();throw;}
}
HarryUpAnimationBinding::HarryUpAnimationBinding(TimerBackend& value,const FontRom& font,std::function<void(unsigned)> wait):backend(value),previous(value.harryup_animation) {
	if(!wait) throw std::invalid_argument("Text animation timing/presentation is not bound");
	backend.harryup_animation=[&fb=value.framebuffer,&font,wait=std::move(wait)]() {harryup_text_animate(fb,font,wait);};
}
HarryUpAnimationBinding::~HarryUpAnimationBinding() {backend.harryup_animation=std::move(previous);}
}
