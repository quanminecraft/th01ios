#pragma once
#include "boss_entity_backend.hpp"
#include "boss_hit_backend.hpp"
#include "hp_backend.hpp"
#include "th01/math/clamp.hpp"
#include "th01/main/playfld.hpp"
struct point_t {pixel_t x{},y{};};
using singyoku_int_t=std::int16_t;
#define inhibit_Z3(value) (value)
#define singyoku_comp(color,i) (color).component(i)
#define z_Palettes native_bomb_palette()
#define stage_palette native_bomb_stage_palette()
namespace th01_native {
struct SingyokuBackend {
	std::function<void(unsigned)> wait;
	std::function<void()> music_fade,defeat;
	Palette4 boss_palette;
};
class SingyokuBinding {
	SingyokuBackend* previous;
public:
	explicit SingyokuBinding(SingyokuBackend& backend);
	~SingyokuBinding();
	SingyokuBinding(const SingyokuBinding&)=delete;
	SingyokuBinding& operator=(const SingyokuBinding&)=delete;
};
}
void singyoku_load();void singyoku_main();void singyoku_free();
int native_singyoku_phase();int native_singyoku_hp();int native_singyoku_frame();
void frame_delay(unsigned frames);void stage_palette_set(const Palette4& palette);void boss_palette_snap();
void mdrv2_bgm_fade_out_nonblock();void singyoku_defeat_animate_and_select_route();
void native_singyoku_defeat_dispatch();
inline void select_for_rank(singyoku_int_t& ret,int easy,int normal,int hard,int lunatic) {ret=rank==RANK_EASY ? easy : rank==RANK_HARD ? hard : rank==RANK_LUNATIC ? lunatic : normal;}
inline void select_subpixel_for_rank(subpixel_t& ret,float easy,float normal,float hard,float lunatic) {select_for_rank(ret,to_sp(easy),to_sp(normal),to_sp(hard),to_sp(lunatic));}
