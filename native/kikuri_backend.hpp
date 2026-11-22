#pragma once
#include "laser_backend.hpp"
#include "boss_hit_backend.hpp"
#include "boss_entity_backend.hpp"
#include "hp_backend.hpp"
#include "kikuri_ripple.hpp"
#include "assets.hpp"
#include "th01/main/playfld.hpp"
#include "th01/hardware/grpinv32.hpp"
namespace th01_native {
struct KikuriBackend {
    Framebuffer& framebuffer;
    const std::vector<Entry>& archive;
    SpriteSet& ripple_sprites;
    std::function<void(unsigned)> wait;
    std::function<void()> music_fade,defeat;
    bool defeated=false;
    int subphase=0,patterns_done=0;
};
class KikuriBinding {
    KikuriBackend* previous;
public:
    explicit KikuriBinding(KikuriBackend& value);
    ~KikuriBinding();
    KikuriBinding(const KikuriBinding&)=delete;
    KikuriBinding& operator=(const KikuriBinding&)=delete;
};
}
namespace kikuri_original {
using kikuri_int_t=std::int16_t;
extern kikuri_int_t boss_hp,boss_phase_frame;
extern std::int8_t kikuri_phase;
extern Palette4 boss_palette;
struct RGB4 {static constexpr int max() {return 15;}};
void kikuri_load();void kikuri_main();void kikuri_free();bool16 kikuri_hittest_orb();unsigned native_kikuri_tears_alive();
void native_kikuri_reset_backend();bool native_kikuri_before_frame();void native_kikuri_observe(int subphase,int patterns);
void native_kikuri_copy_line(screen_y_t y,std::uint16_t mask);
void ptn_load(main_ptn_slot_t slot,const char* name);void ptn_free(main_ptn_slot_t slot);
void frame_delay(unsigned frames);void mdrv2_bgm_fade_out_nonblock();void boss_defeat_animate();
void boss_palette_snap();void boss_palette_show();void stage_palette_set(const Palette4& value);
Palette4& native_kikuri_stage_palette();
inline void player_put_default() {ptn_put_8(player_left,player_top,PTN_MIKO_L);}
inline void orb_put_default() {ptn_put_8(orb_cur_left,orb_cur_top,PTN_ORB);}
inline void select_for_rank(int& value,int easy,int normal,int hard,int lunatic) {value=rank==RANK_EASY ? easy : rank==RANK_HARD ? hard : rank==RANK_LUNATIC ? lunatic : normal;}
inline void select_for_rank(std::int16_t& value,int easy,int normal,int hard,int lunatic) {int n;select_for_rank(n,easy,normal,hard,lunatic);value=n;}
inline void select_for_rank(pellet_group_t& value,int easy,int normal,int hard,int lunatic) {int n;select_for_rank(n,easy,normal,hard,lunatic);value=static_cast<pellet_group_t>(n);}
inline void select_subpixel_for_rank(subpixel_t& value,float easy,float normal,float hard,float lunatic) {value=to_sp(rank==RANK_EASY ? easy : rank==RANK_HARD ? hard : rank==RANK_LUNATIC ? lunatic : normal);}
inline void select_laser_speed_for_rank(std::int16_t& value,float easy,float normal,float hard,float lunatic) {select_subpixel_for_rank(value,easy/2,normal/2,hard/2,lunatic/2);}
}
#define stage_palette kikuri_original::native_kikuri_stage_palette()
#define z_Palettes native_bomb_palette()
#define kikuri_component(color,i) (color).component(i)
#define ptn_sloppy_unput_before_alpha_put native_kikuri_ripple_sloppy()
#define ptn_put_quarter_8 native_kikuri_ripple_put