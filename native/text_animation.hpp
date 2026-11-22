#pragma once
#include "timer_backend.hpp"
namespace th01_native {
void scene_title_text_animate(Framebuffer& framebuffer,const FontRom& font,const std::function<void(unsigned)>& wait);
void harryup_text_animate(Framebuffer& framebuffer,const FontRom& font,const std::function<void(unsigned)>& wait);
void stage_number_text_animate(Framebuffer& framebuffer,const FontRom& font,unsigned stage,const std::function<void(unsigned)>& wait);
class HarryUpAnimationBinding {
	TimerBackend& backend;
	std::function<void()> previous;
public:
	HarryUpAnimationBinding(TimerBackend& backend,const FontRom& font,std::function<void(unsigned)> wait);
	~HarryUpAnimationBinding();
	HarryUpAnimationBinding(const HarryUpAnimationBinding&)=delete;
	HarryUpAnimationBinding& operator=(const HarryUpAnimationBinding&)=delete;
};
}
