#pragma once
#include "missile_backend.hpp"
#include "laser_backend.hpp"
#include "boss_hit_backend.hpp"
#include "boss_entity_backend.hpp"
#include "hp_backend.hpp"
#include "line_backend.hpp"
#include "th01/main/particle.hpp"
#include "th01/main/shape.hpp"
#include "th01/sprites/pillar.hpp"
#include "th01/math/dir.hpp"
namespace th01_native {
struct MimaBackend {
    Framebuffer& framebuffer;
    MissileBackend& missiles;
    std::uint32_t& frame;
    std::function<void(unsigned)> wait,delay_ms;
    std::function<void()> music_fade,defeat;
    Palette4 boss_palette;
    std::array<std::vector<Bytes>,8> tiles;
    bool defeated=false;
    int pattern=0,invincibility=0;
};
class MimaBinding {
    MimaBackend* previous;
public:
    explicit MimaBinding(MimaBackend& value);
    ~MimaBinding();
    MimaBinding(const MimaBinding&)=delete;
    MimaBinding& operator=(const MimaBinding&)=delete;
};
}
namespace mima_original {
using mima_int_t=std::int16_t;
extern mima_int_t boss_hp,boss_phase_frame;
extern std::int8_t boss_phase;
constexpr int ROW_SIZE=80,V_RED=2;
using screen_point_t=th01_native::MimaPoint;using vram_offset_t=std::int16_t;
void mima_load();void mima_main();void mima_free();
void native_mima_reset_backend();bool native_mima_before_frame();void native_mima_observe(int pattern,int invincibility);
void ptn_new(int slot,int count);void ptn_free(int slot);
void ptn_snap_rect_from_1_8(screen_x_t left,screen_y_t top,int w,int h,int slot,int& image,int& x,int& y);
void ptn_put_rect_noalpha_8(screen_x_t left,screen_y_t top,int w,int h,int slot,int& image,int& x,int& y);
void ptn_put_row_noalpha_8(screen_x_t left,screen_y_t top,int w,int slot,int& image,int& x);
void ptn_put_quarter_noalpha_8(screen_x_t left,screen_y_t top,int id,int quarter);
void graph_r_lineloop_put(const screen_point_t* points,int count,int color);void graph_r_lineloop_unput(const screen_point_t* points,int count);
void grcg_setcolor_rmw(int color);void grcg_off_func();void grcg_put(vram_offset_t offset,std::uint32_t mask,int width);
vram_offset_t vram_offset_shift(int x,int y);
void frame_delay(unsigned frames);void z_palette_white_in();void mdrv2_bgm_fade_out_nonblock();void boss_defeat_animate();
#define stage_palette native_bomb_stage_palette()
#define z_Palettes native_bomb_palette()
#define graph_r_line_unput native_graph_line_unput
inline void select_for_rank(int& value,int easy,int normal,int hard,int lunatic) {value=rank==RANK_EASY ? easy : rank==RANK_HARD ? hard : rank==RANK_LUNATIC ? lunatic : normal;}
inline void select_for_rank(std::int16_t& value,int easy,int normal,int hard,int lunatic) {int n;select_for_rank(n,easy,normal,hard,lunatic);value=n;}
inline void select_subpixel_for_rank(subpixel_t& value,float easy,float normal,float hard,float lunatic) {value=to_sp(rank==RANK_EASY ? easy : rank==RANK_HARD ? hard : rank==RANK_LUNATIC ? lunatic : normal);}
inline void select_laser_speed_for_rank(std::int16_t& value,float easy,float normal,float hard,float lunatic) {select_subpixel_for_rank(value,easy/2,normal/2,hard/2,lunatic/2);}
}