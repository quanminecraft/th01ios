#include "mima_hop.hpp"
#include "boss_entity_backend.hpp"
#include "bomb_backend.hpp"
#include "th01/math/dir.hpp"
namespace {
constexpr int MIMA_W=128,MIMA_H=160,BASE_TOP=120,PLAYFIELD_W=640,PLAYFIELD_H=336;
struct Runtime {
    CBossEntitySized<MIMA_W,MIMA_H> entity{};
    std::int16_t frame=0;
    std::uint8_t interval=4,speed=16;
    struct {subpixel_t speed=0;} pattern;
    bool meteor=true;
    std::array<std::uint8_t,MIMA_W*MIMA_H> background{};
    unsigned snapshots=0;
};
Runtime* active=nullptr;
Runtime& current() {if(!active) throw std::logic_error("Mima hop is not bound");return *active;}
#define ent_still current().entity
#define boss_phase_frame current().frame
#define spreadin_interval current().interval
#define spreadin_speed current().speed
#define pattern_state current().pattern
#define meteor_active current().meteor
#define z_Palettes native_bomb_palette()
#define stage_palette native_bomb_stage_palette()
void select_for_rank(int& value,int easy,int normal,int hard,int lunatic) {value=rank==RANK_EASY ? easy : rank==RANK_HARD ? hard : rank==RANK_LUNATIC ? lunatic : normal;}
void select_subpixel_for_rank(subpixel_t& value,float easy,float normal,float hard,float lunatic) {value=to_sp(rank==RANK_EASY ? easy : rank==RANK_HARD ? hard : rank==RANK_LUNATIC ? lunatic : normal);}
void mima_bg_snap() {
    auto& r=current();auto& fb=th01_native::graphics_framebuffer();
    for(int y=0;y<MIMA_H;++y) for(int x=0;x<MIMA_W;++x) r.background[y*MIMA_W+x]=fb.pixel(1,ent_still.cur_left+x,ent_still.cur_top+y);
    ++r.snapshots;
}
void mima_unput_both() {
    auto& r=current();auto& fb=th01_native::graphics_framebuffer();
    for(unsigned page=0;page<2;++page) for(int y=0;y<MIMA_H;++y) for(int x=0;x<MIMA_W;++x) fb.put(page,ent_still.cur_left+x,ent_still.cur_top+y,r.background[y*MIMA_W+x]);
    page_access(0);
}
void mima_put_still_both() {page_access(1);ent_still.put_8();page_access(0);ent_still.put_8();}
#include "th01/main/boss/mima_spreadin.inc"
#include "th01/main/boss/mima_vertical_transition.inc"
#include "th01/main/boss/mima_hop.inc"
#undef ent_still
#undef boss_phase_frame
#undef spreadin_interval
#undef spreadin_speed
#undef pattern_state
#undef meteor_active
#undef z_Palettes
#undef stage_palette
}
namespace th01_native {
struct MimaHopPattern::Impl {
    Runtime runtime;
    bool done=false;
    Impl() {
        if(active) throw std::logic_error("Original Mima hop supports one active instance");
        active=&runtime;
        try {
            runtime.entity.load("boss3_1.bos",0);runtime.entity.set_image(0);runtime.entity.pos_cur_set(256,120);
            mima_bg_snap();mima_put_still_both();pattern_hop_and_fire_chase_pellets(false);
        } catch(...) {active=nullptr;throw;}
    }
    ~Impl() {active=nullptr;}
};
MimaHopPattern::MimaHopPattern():impl(std::make_unique<Impl>()) {}
MimaHopPattern::~MimaHopPattern()=default;
bool MimaHopPattern::advance() {
    if(impl->done) return true;
    ++impl->runtime.frame;pattern_hop_and_fire_chase_pellets();impl->done=impl->runtime.frame==0;return impl->done;
}
void MimaHopPattern::restart() {if(!impl->done) throw std::logic_error("Mima hop is still running");impl->done=false;}
MimaHopState MimaHopPattern::state() const {const auto& r=impl->runtime;return {r.entity.cur_left,r.entity.cur_top,r.frame,bool(r.entity.hitbox_orb_inactive),r.meteor,r.snapshots};}
}