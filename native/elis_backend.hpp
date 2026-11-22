#pragma once
#include "laser_backend.hpp"
#include "missile_backend.hpp"
#include "boss_hit_backend.hpp"
#include "boss_entity_backend.hpp"
#include "hp_backend.hpp"
#include "line_backend.hpp"
#include "th01/main/entity.hpp"
#include "th01/main/shape.hpp"
#include "th01/math/dir.hpp"
namespace th01_native {
struct ElisBackend{
    Framebuffer& framebuffer;const std::vector<Entry>& archive;MissileBackend& missiles;
    std::function<void(unsigned)> wait,delay_ms;std::function<void()> music_fade,defeat;
    Palette4 palette{};SpriteSet grc;std::array<std::vector<Bytes>,8> tiles;
    bool defeated=false,reset_pending=true,bat=false;int pattern=0,invincibility=0,bat_pattern=0,girl_pattern=0,subphase=0;
};
class ElisBinding{ElisBackend* previous;public:explicit ElisBinding(ElisBackend&);~ElisBinding();ElisBinding(const ElisBinding&)=delete;ElisBinding& operator=(const ElisBinding&)=delete;};
}
namespace elis_original {
using elis_int_t=std::int16_t;constexpr int V_WHITE=7;
extern elis_int_t boss_hp,boss_phase_frame;extern std::int8_t boss_phase;
void elis_load();void elis_setup();void elis_free();void elis_main();
void native_elis_reset();bool native_elis_before_frame();bool native_elis_take_reset();void native_elis_observe(bool bat,int pattern,int invincibility);
void native_elis_observe_girl(int);void native_elis_observe_final(int,int);
void ptn_new(int slot,int count);void ptn_free(int slot);
void ptn_snap_rect_from_1_8(screen_x_t,screen_y_t,int,int,int,int&,int&,int&);
void ptn_put_row_noalpha_8(screen_x_t,screen_y_t,int,int,int&,int&);
void ptn_put_rect_noalpha_8(screen_x_t,screen_y_t,int,int,int,int&,int&,int&);
void grc_load(int,const char*);void grc_free(int);void native_elis_grc_put(screen_x_t,screen_y_t,int,int,unsigned);
void frame_delay(unsigned);void delay(unsigned);void mdrv2_bgm_fade_out_nonblock();void boss_defeat_animate();void boss_palette_snap();void boss_palette_show();
inline void select_for_rank(int& r,int e,int n,int h,int l){r=rank==RANK_EASY ? e:rank==RANK_HARD ? h:rank==RANK_LUNATIC ? l:n;}
inline void select_for_rank(std::int16_t& r,int e,int n,int h,int l){int v;select_for_rank(v,e,n,h,l);r=v;}
inline void select_for_rank(pellet_group_t& r,int e,int n,int h,int l){int v;select_for_rank(v,e,n,h,l);r=static_cast<pellet_group_t>(v);}
inline void select_laser_speed_for_rank(int& r,float e,float n,float h,float l){r=to_sp((rank==RANK_EASY ? e:rank==RANK_HARD ? h:rank==RANK_LUNATIC ? l:n)/2);}
}
#define grc_put_8 native_elis_grc_put
#undef grc_sloppy_unput
#define grc_sloppy_unput(left,top) egc_copy_rect_1_to_0_16(left,top,48,32);
#define frame_rand native_boss_frame()
#define random_seed native_random_seed
#define graph_r_line_unput native_graph_line_unput