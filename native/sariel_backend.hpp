#pragma once
#include "laser_backend.hpp"
#include "boss_hit_backend.hpp"
#include "boss_entity_backend.hpp"
#include "hp_backend.hpp"
#include "line_backend.hpp"
#include "th01/main/playfld.hpp"
#include "th01/main/entity.hpp"
#include "th01/main/shape.hpp"
#include "th01/math/dir.hpp"
#include "th01/hardware/grpinv32.hpp"
#include "th01/main/spawnray.hpp"
#include "th01/sprites/leaf.hpp"
#include "th01/sprites/shape8x8.hpp"
#include <cmath>
#include <filesystem>
namespace sariel_original {
using std::abs;
using sariel_frame_t=std::int16_t;
using dots8_t=std::uint8_t;using dots16_t=std::uint16_t;
using vram_offset_t=std::int16_t;using vram_word_amount_t=std::int16_t;using vram_byte_amount_t=std::int16_t;
using svc_t=std::int8_t;
constexpr int V_WHITE=7,ROW_SIZE=80,PLANE_SIZE=32000,BYTE_MASK=7,TX_BLACK=0,TX_REVERSE=1,TX_WHITE=7,GPF_PALETTE_SHOW=1,GPF_PALETTE_KEEP=0;
constexpr const char* SE_FN="zigoku.mde";
struct RGB4{static constexpr int max(){return 15;}};
struct screen_point_t {screen_x_t x;screen_y_t y;};
struct point_t {pixel_t x;pixel_t y;};
template<class T,int H>struct DotRect{T row[H];T& operator[](int i){return row[i];}};
struct NativeLeafSprite{dots8_t row[8];};
struct LoadedPalette{struct Color{std::int16_t v[3]{};};std::array<Color,16> colors{};Color& operator[](unsigned i){return colors.at(i);}const Color& operator[](unsigned i)const{return colors.at(i);}};
extern sariel_frame_t boss_hp,boss_phase_frame;extern std::int8_t boss_phase;extern bool game_cleared;
void sariel_entrance(std::int8_t);void sariel_load_and_init();void sariel_setup();void sariel_main();void sariel_free();
void frame_delay(unsigned);void delay(unsigned);
void boss_palette_snap();void boss_palette_show();void stage_palette_set(const Palette4&);
LoadedPalette& native_sariel_grp_palette();
void native_sariel_black_out_step();
void native_sariel_reset();void native_sariel_observe(int,int,int,bool);
extern bool native_actor_reset;
void text_fillca(char,int);void stageobjs_init_and_render(int);
void mdrv2_bgm_load(const char*);void mdrv2_bgm_play();void mdrv2_bgm_stop();void mdrv2_bgm_fade_out_nonblock();void mdrv2_se_load(const char*);
void grp_put(const char*,int);void hud_rerender();void hud_hp_rerender(int);
void pagetrans_diagonal_8x8(unsigned);void pagetrans_diagonal_8x8_with_palette(unsigned,Palette4&,const LoadedPalette&);
struct EGCCopy{void rect_interpage(int,int,int,int,unsigned);};
void grc_load(int,const char*);void grc_free(int);void native_sariel_grc_put(screen_x_t,screen_y_t,int,int,unsigned);
void ptn_new(int,int);void ptn_free(int);
void ptn_snap_rect_from_1_8(screen_x_t,screen_y_t,int,int,int,int&,int&,int&);
void native_sariel_tiles_put(screen_x_t,screen_y_t,int,int,int,int&,int&,int&);
struct GrcgChunk{int offset,width;operator std::uint16_t()const;void operator=(std::uint16_t)const;};
GrcgChunk grcg_chunk(int,int);dots8_t grcg_chunk_8(int);void grcg_put_emptyopt(int,dots8_t,int);
void grcg_setcolor_tcr(unsigned);void grcg_setcolor_rmw(unsigned);void grcg_off_func();
void grcg_put_8x8_mono(vram_offset_t,char,const dots8_t*,std::uint8_t);
template<class T>vram_offset_t vram_offset_divmul(T x,T y){return static_cast<vram_offset_t>((x/8)+(y*80));}
inline vram_offset_t vram_offset_shift(int x,int y){return std::int16_t((x>>3)+y*80);}
inline void select_for_rank(int& value,int easy,int normal,int hard,int lunatic){value=rank==RANK_EASY ? easy:rank==RANK_HARD ? hard:rank==RANK_LUNATIC ? lunatic:normal;}
inline void select_for_rank(std::int16_t& value,int easy,int normal,int hard,int lunatic){int result;select_for_rank(result,easy,normal,hard,lunatic);value=result;}
inline void select_laser_speed_for_rank(std::int16_t& value,float easy,float normal,float hard,float lunatic){value=to_sp((rank==RANK_EASY ? easy:rank==RANK_HARD ? hard:rank==RANK_LUNATIC ? lunatic:normal)/2);}
inline screen_x_t polar_x(int center,int radius,unsigned char angle){return ::polar_x(pixel_t(center),pixel_t(radius),angle);}
inline screen_y_t polar_y(int center,int radius,unsigned char angle){return ::polar_y(pixel_t(center),pixel_t(radius),angle);}
extern bool native_particle_reset;
void native_sariel_bind_particles(unsigned,std::function<std::array<double,5>(unsigned)>);
void native_sariel_bind_leaves(std::function<std::array<int,5>(unsigned)>);
template<class C,class P,class V>void native_particle_bind(unsigned kind,C* col,P* left,P* top,V* velocity,const int* age){native_sariel_bind_particles(kind,[=](unsigned i){return std::array<double,5>{double(col[i]),double(left[i]),double(top[i]),double(velocity[i]),double(age ? age[i]:0)};});}
template<class F>void native_leaf_bind(F* flags,DecimalSubpixel* left,DecimalSubpixel* top,DecimalSubpixel* vx,DecimalSubpixel* vy){native_sariel_bind_leaves([=](unsigned i){return std::array<int,5>{int(flags[i]),left[i].v,top[i].v,vx[i].v,vy[i].v};});}
}
namespace th01_native {
struct SarielBackend {
 Framebuffer& framebuffer;const std::vector<Entry>& archive;std::filesystem::path assets;
 std::function<void(unsigned)> wait,delay_ms;
 std::function<void()> stage_reset,hud;
 std::function<void(const std::string&,const std::string&)> audio;
 Palette4 palette{};sariel_original::LoadedPalette loaded{};
 std::array<SpriteSet,4> grc;std::array<std::vector<Bytes>,8> tiles;
 std::array<std::function<std::array<double,5>(unsigned)>,3> particles;
 std::function<std::array<int,5>(unsigned)> leaves;
 std::vector<std::string> missing_images;
 unsigned color=0;int pattern=0,patterns_done=0,invincibility=0;bool invincible=false;
};
class SarielBinding {SarielBackend* previous;public:explicit SarielBinding(SarielBackend&);~SarielBinding();SarielBinding(const SarielBinding&)=delete;SarielBinding& operator=(const SarielBinding&)=delete;};
}

#define z_Palettes native_bomb_palette()
#define grp_palette native_sariel_grp_palette()
#define z_palette_black_out_step_bugged(col,comp) native_sariel_black_out_step();
#define grc_put_8 native_sariel_grc_put
#undef grc_sloppy_unput
#define grc_sloppy_unput(left,top) egc_copy_rect_1_to_0_16(left,top,48,32);
#define ptn_put_rect_noalpha_8(...) native_sariel_tiles_put(__VA_ARGS__);
#define graph_r_line_unput native_graph_line_unput
#define inhibit_Z3(value) (value)
#define random_seed native_random_seed
#define frame_rand native_boss_frame()
