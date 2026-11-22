#pragma once
#include "physics_types.hpp"
#include "framebuffer.hpp"
#include <functional>
#include "th01/sprites/main_ptn.h"

namespace th01_native {
// Original rendering and inspectable sound commands. An optional dispatcher
// sends each SE immediately to the native audio device, including cutscenes.
struct ShotBackend {
	Framebuffer& framebuffer;
	const SpriteSet& sprites;
	std::vector<unsigned> sound_commands;
	unsigned page=0;
 std::function<void(unsigned)> sound_dispatch;
};
unsigned current_render_page();
Framebuffer& graphics_framebuffer();
class ShotBackendBinding {
	ShotBackend* previous;
public:
	explicit ShotBackendBinding(ShotBackend& backend);
	~ShotBackendBinding();
	ShotBackendBinding(const ShotBackendBinding&)=delete;
	ShotBackendBinding& operator=(const ShotBackendBinding&)=delete;
};
}
void ptn_sloppy_unput_quarter_16(screen_x_t left,screen_y_t top);
void egc_copy_rect_1_to_0_16(screen_x_t left,screen_y_t top,pixel_t width,pixel_t height);
void ptn_put_quarter_8(screen_x_t left,screen_y_t top,int id,int quarter);
void ptn_unput_quarter_8(screen_x_t left,screen_y_t top,int id,int quarter);
void mdrv2_se_play(int id);
void page_access(unsigned page);
void ptn_sloppy_unput_16(screen_x_t left,screen_y_t top);
void egc_copy_rect_1_to_0_16_word_w(screen_x_t left,screen_y_t top,pixel_t width,pixel_t height);
