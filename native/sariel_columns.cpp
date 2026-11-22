#include "sariel_columns.hpp"
#include "pellet_backend.hpp"
#include "line_backend.hpp"
#include "th01/main/playfld.hpp"
#include "th01/math/dir.hpp"
#include "th01/main/spawnray.hpp"
namespace {
using sariel_frame_t=int;
constexpr int RES_X=640,RES_Y=400,GRC_SLOT_VORTEX_DEBRIS=0;
int boss_phase_frame=0;
union {int interval;int pellet_count;} pattern_state{};
th01_native::SpriteSet* sprites=nullptr;
#include "th01/main/boss/sariel_coords.inc"
void select_for_rank(int& value,int easy,int normal,int hard,int lunatic){value=rank==RANK_EASY ? easy:rank==RANK_HARD ? hard:rank==RANK_LUNATIC ? lunatic:normal;}
void grc_put_8(int left,int top,int,int cel,unsigned color){th01_native::graphics_framebuffer().blit(th01_native::current_render_page(),left,top,sprites->images.at(cel),true,color);}
#define grc_sloppy_unput(left,top) egc_copy_rect_1_to_0_16(left,top,48,32);
#include "th01/main/boss/sariel_vortex_sprites.inc"
#define graph_r_line_unput native_graph_line_unput
#define inhibit_Z3(value) (value)
#include "th01/main/boss/sariel_spawnrays.inc"
#undef inhibit_Z3
#include "th01/main/boss/sariel_columns.inc"
#include "th01/main/boss/sariel_seal_rain.inc"
}
namespace th01_native {
struct SarielColumns::Impl {
 SarielColumnKind kind;SpriteSet images;int frame=0;bool done=false;
 Impl(const std::vector<Entry>& archive,std::uint32_t seed,SarielColumnKind selected):kind(selected),images(read_sprites(asset(archive,"BOSS6GR1.GRC"))){if(sprites)throw std::logic_error("Sariel columns support one instance");sprites=&images;spawnray_reset();native_random_seed=seed;}
 ~Impl(){sprites=nullptr;}
};
SarielColumns::SarielColumns(const std::vector<Entry>& archive,std::uint32_t seed,SarielColumnKind kind):impl(std::make_unique<Impl>(archive,seed,kind)){}
SarielColumns::~SarielColumns()=default;
bool SarielColumns::advance(){auto& r=*impl;if(r.done)return true;boss_phase_frame=++r.frame;if(r.kind==SarielColumnKind::SealRain)pattern_rain_from_seal_center(boss_phase_frame);else pattern_vertical_stacks_from_bottom_then_random_rain_from_top();r.frame=boss_phase_frame;r.done=r.frame==0;return r.done;}
int SarielColumns::frame()const{return impl->frame;}
}
