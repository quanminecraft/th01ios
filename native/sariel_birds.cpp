#include "sariel_birds.hpp"
#include "pellet_backend.hpp"
#include "shape_types.hpp"
#include "line_backend.hpp"
#include "th01/main/playfld.hpp"
#include "th01/math/dir.hpp"
#include "th01/main/spawnray.hpp"
#include "th01/main/entity.hpp"
#include <chrono>
#include <thread>
namespace {
constexpr bool native_actor_reset=false;
constexpr int RES_X=640,RES_Y=400,BIRD_COUNT=30,BIRD_W=32,BIRD_H=32,BIRD_HATCH_CELS=5,BIRD_FLY_CELS=2,C_HATCH=1,C_FLY=6;
int boss_phase_frame=0;
th01_native::SpriteSet* sprites=nullptr;
std::function<void(unsigned)>* delay_callback=nullptr;
void delay(unsigned ms){if(delay_callback && *delay_callback)(*delay_callback)(ms);else std::this_thread::sleep_for(std::chrono::milliseconds(ms));}
void bird_put(int left,int top,int cel){auto& fb=th01_native::graphics_framebuffer();fb.blit(th01_native::current_render_page(),left,top,sprites->images.at(cel),true,15);}
#define bird_put_8(left,top,cel) bird_put(left,top,cel);
#define grc_sloppy_unput(left,top) egc_copy_rect_1_to_0_16(left,top,48,32);
#include "th01/main/boss/sariel_birds.inc"
#include "th01/main/boss/sariel_coords.inc"
constexpr int SPAWNCROSS_W=32,SPAWNCROSS_H=32,SPAWNCROSS_CELS=2,GRC_SLOT_SPAWNCROSS=0,C_EGG=0;
struct point_t {pixel_t x,y;};
union {int interval;int unknown;} pattern_state{};
th01_native::SarielBody* active_body=nullptr;
th01_native::SpriteSet* cross_sprites=nullptr;
th01_native::SpriteSet* debris_sprites=nullptr;
void select_for_rank(int& value,int easy,int normal,int hard,int lunatic){value=rank==RANK_EASY ? easy:rank==RANK_HARD ? hard:rank==RANK_LUNATIC ? lunatic:normal;}
bool wand_render_raise_both(){return active_body->raise_wand();}
void wand_lower_both(){active_body->lower_wand();}
void grc_put_8(int left,int top,int slot,int cel,unsigned color){th01_native::graphics_framebuffer().blit(th01_native::current_render_page(),left,top,(slot==0 ? cross_sprites:debris_sprites)->images.at(cel),true,color);}
#include "th01/main/boss/sariel_bird_arc.inc"
constexpr int GRC_SLOT_VORTEX_DEBRIS=1;
#include "th01/main/boss/sariel_vortex_sprites.inc"
#define graph_r_line_unput native_graph_line_unput
#define inhibit_Z3(value) (value)
#include "th01/main/boss/sariel_spawnrays.inc"
#undef inhibit_Z3
#include "th01/main/boss/sariel_bottom_birds.inc"

}
namespace th01_native {
struct SarielBirds::Impl {
 SpriteSet images,cross,debris;std::function<void(unsigned)> delay;int arc_frame=0;bool arc_started=false,arc_done=false,bottom_started=false;int bottom_frame=0;bool bottom_done=false;
 Impl(const std::vector<Entry>& archive,std::function<void(unsigned)> callback):images(read_sprites(asset(archive,"BOSS6GR2.GRC"))),cross(read_sprites(asset(archive,"BOSS6GR3.GRC"))),debris(read_sprites(asset(archive,"BOSS6GR1.GRC"))),delay(std::move(callback)){if(sprites)throw std::logic_error("Sariel birds support one flock");Birds={};sprites=&images;delay_callback=&delay;cross_sprites=&cross;debris_sprites=&debris;}
 ~Impl(){Birds.reset();sprites=nullptr;delay_callback=nullptr;cross_sprites=nullptr;debris_sprites=nullptr;active_body=nullptr;}
};
SarielBirds::SarielBirds(const std::vector<Entry>& archive,std::function<void(unsigned)> delay):impl(std::make_unique<Impl>(archive,std::move(delay))){}
SarielBirds::~SarielBirds()=default;
void SarielBirds::reset(){Birds.reset();}
void SarielBirds::spawn(double left,double top,double vx,double vy){Birds.spawn(left,top,vx,vy);}
void SarielBirds::fire(SarielBirdFire group){switch(group){case SarielBirdFire::Aimed:Birds.fire(BPG_AIMED);break;case SarielBirdFire::Ring:Birds.fire(BPG_6_RING);break;case SarielBirdFire::Rain:Birds.fire(BPG_RANDOM_RAIN);break;}}
void SarielBirds::update(int frame){if(frame<0 || frame>32767)throw std::invalid_argument("Sariel bird frame");boss_phase_frame=frame;Birds.unput_update_render();}
void SarielBirds::begin_arc(SarielBody& body,std::uint32_t seed){auto& r=*impl;r.bottom_started=false;r.arc_frame=0;r.arc_done=false;r.arc_started=true;Birds.reset();active_body=&body;body.raise_wand(true);native_random_seed=seed;}
bool SarielBirds::advance_arc(){auto& r=*impl;if(!r.arc_started)throw std::logic_error("Sariel bird arc has not begun");if(r.arc_done)return true;boss_phase_frame=++r.arc_frame;Birds.unput_update_render();pattern_birds_on_ellipse_arc();r.arc_frame=boss_phase_frame;r.arc_done=r.arc_frame==0;return r.arc_done;}
int SarielBirds::arc_frame()const{return impl->arc_frame;}
void SarielBirds::begin_bottom(std::uint32_t seed){auto& r=*impl;r.arc_started=false;r.bottom_started=true;r.bottom_frame=0;r.bottom_done=false;Birds.reset();spawnray_reset();native_random_seed=seed;}
bool SarielBirds::advance_bottom(){auto& r=*impl;if(!r.bottom_started)throw std::logic_error("Sariel bottom birds have not begun");if(r.bottom_done)return true;boss_phase_frame=++r.bottom_frame;Birds.unput_update_render();pattern_symmetric_birds_from_bottom();r.bottom_frame=boss_phase_frame;r.bottom_done=r.bottom_frame==0;return r.bottom_done;}
int SarielBirds::bottom_frame()const{return impl->bottom_frame;}
SarielBirdState SarielBirds::state(unsigned i)const{if(i>=30)throw std::out_of_range("Sariel bird slot");return{Birds.alive[i]!=0,Birds.hatch_time[i],Birds.left[i],Birds.top[i],Birds.velocity_x[i],Birds.velocity_y[i]};}
}
