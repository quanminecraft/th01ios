#include "singyoku_backend.hpp"
namespace {
th01_native::SingyokuBackend* active=nullptr;
th01_native::SingyokuBackend& backend() {if(!active) throw std::runtime_error("Singyoku host is not bound");return *active;}
}
namespace th01_native {
SingyokuBinding::SingyokuBinding(SingyokuBackend& value):previous(active) {
	if(!value.wait || !value.music_fade || !value.defeat) throw std::invalid_argument("Singyoku timing/music/defeat callbacks are required");active=&value;
}
SingyokuBinding::~SingyokuBinding() {active=previous;}
}
void frame_delay(unsigned frames) {backend().wait(frames);}
void stage_palette_set(const Palette4& palette) {
	// Stage color storage is mutable in the original game.
	const_cast<Palette4&>(native_bomb_stage_palette())=palette;
}
void boss_palette_snap() {backend().boss_palette=native_bomb_palette();}
void mdrv2_bgm_fade_out_nonblock() {backend().music_fade();}
void native_singyoku_defeat_dispatch() {backend().defeat();}
