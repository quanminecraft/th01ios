#pragma once
#include "hud_mask.hpp"
#include "hud_score.hpp"
#include "timer_backend.hpp"
namespace th01_native {
struct HudLayoutBackend {Framebuffer& framebuffer;const HudMask& mask;};
class HudLayoutBinding {
	HudLayoutBackend* previous;
public:
	explicit HudLayoutBinding(HudLayoutBackend& backend);
	~HudLayoutBinding();
	HudLayoutBinding(const HudLayoutBinding&)=delete;
	HudLayoutBinding& operator=(const HudLayoutBinding&)=delete;
};
}
void native_hud_layout_render();
