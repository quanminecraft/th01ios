#include "sariel_vortices.hpp"
#include "pellet_backend.hpp"
#include "shape_types.hpp"
#include "th01/main/playfld.hpp"
#include "th01/main/entity.hpp"
#include "th01/math/dir.hpp"
namespace {
constexpr bool native_actor_reset=false;
struct Runtime {th01_native::SarielBody& body;th01_native::SpriteSet sprites;int frame=0;bool done=false;};
Runtime* active=nullptr;
Runtime& current(){if(!active)throw std::logic_error("Sariel vortices are not bound");return *active;}
int boss_phase_frame=0;
struct {int frame=0;} pattern_state;
constexpr int VORTEX_COUNT=2,VORTEX_W=32,VORTEX_H=32,VORTEX_CELS=3,C_VORTEX=0;
void select_for_rank(int& value,int easy,int normal,int hard,int lunatic){value=rank==RANK_EASY ? easy:rank==RANK_HARD ? hard:rank==RANK_LUNATIC ? lunatic:normal;}
bool wand_render_raise_both(){return current().body.raise_wand();}
void wand_lower_both(){current().body.lower_wand();}
void vortex_put(int left,int top,int cel){auto& sprite=current().sprites.images.at(cel);auto& fb=th01_native::graphics_framebuffer();fb.blit(th01_native::current_render_page(),left,top,sprite,true,7);}
#define vortex_or_debris_put_8(left,top,cel) vortex_put(left,top,cel);
#define grc_sloppy_unput(left,top) egc_copy_rect_1_to_0_16(left,top,48,32);
#include "th01/main/boss/sariel_vortices.inc"
}
namespace th01_native {
struct SarielVortices::Impl {
 Runtime runtime;
 Impl(SarielBody& body,const std::vector<Entry>& archive,std::uint32_t seed):runtime{body,read_sprites(asset(archive,"BOSS6GR1.GRC"))}{if(active)throw std::logic_error("Sariel vortices support one instance");body.raise_wand(true);native_random_seed=seed;active=&runtime;}
 ~Impl(){active=nullptr;}
};
SarielVortices::SarielVortices(SarielBody& body,const std::vector<Entry>& archive,std::uint32_t seed):impl(std::make_unique<Impl>(body,archive,seed)){}
SarielVortices::~SarielVortices()=default;
bool SarielVortices::advance(){auto& r=impl->runtime;if(r.done)return true;boss_phase_frame=++r.frame;pattern_vortices();r.frame=boss_phase_frame;r.done=r.frame==0;return r.done;}
int SarielVortices::frame()const{return impl->runtime.frame;}
}
