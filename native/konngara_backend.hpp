#pragma once
#include "laser_backend.hpp"
#include "boss_hit_backend.hpp"
#include "boss_entity_backend.hpp"
#include "hp_backend.hpp"
#include "line_backend.hpp"
#include "konngara_entrance.hpp"
#include "th01/main/playfld.hpp"
#include "th01/main/entity.hpp"
#include "th01/main/shape.hpp"
#include "th01/sprites/shape8x8.hpp"
#include "th01/main/spawnray.hpp"
namespace th01_native {
struct KonngaraBackend {
    KonngaraEntrance& graphics;
    Palette4 palette{};
    int pattern=99,patterns_done=0;
};
class KonngaraBinding {
    KonngaraBackend* previous;
public:
    explicit KonngaraBinding(KonngaraBackend& value);
    ~KonngaraBinding();
    KonngaraBinding(const KonngaraBinding&)=delete;
    KonngaraBinding& operator=(const KonngaraBinding&)=delete;
};
}
namespace konngara_original {
using konngara_int_t=std::int16_t;
using svc2=std::int8_t;
struct point_t {pixel_t x,y;};
extern konngara_int_t boss_hp,boss_phase_frame;
extern std::int8_t boss_phase;
void konngara_load_and_entrance(std::int8_t);void konngara_init();void konngara_main();void konngara_free();
void native_konngara_intro();void native_konngara_defeat();bool native_konngara_before_frame();void native_konngara_observe(int pattern,int done);
void boss_palette_snap();void boss_palette_show();void grx_put(unsigned slot);void grx_free(unsigned slot);
inline void select_for_rank(std::int16_t& value,int easy,int normal,int hard,int lunatic) {value=rank==RANK_EASY ? easy : rank==RANK_HARD ? hard : rank==RANK_LUNATIC ? lunatic : normal;}
inline void select_for_rank(int& value,int easy,int normal,int hard,int lunatic) {value=rank==RANK_EASY ? easy : rank==RANK_HARD ? hard : rank==RANK_LUNATIC ? lunatic : normal;}
inline void select_subpixel_for_rank(subpixel_t& value,float easy,float normal,float hard,float lunatic) {value=to_sp(rank==RANK_EASY ? easy : rank==RANK_HARD ? hard : rank==RANK_LUNATIC ? lunatic : normal);}
}
#define z_Palettes native_bomb_palette()
#define graph_r_line_unput native_graph_line_unput
#define frame_rand native_boss_frame()
#define random_seed native_random_seed
#define far
