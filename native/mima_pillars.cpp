#include "mima_pillars.hpp"
#include "boss_entity_backend.hpp"
#include "pellet_backend.hpp"
#include "line_backend.hpp"
#include "th01/main/playfld.hpp"
#include "th01/main/shape.hpp"
#include "th01/sprites/pillar.hpp"
namespace {
using vram_offset_t=std::int16_t;
constexpr int ROW_SIZE=80,COL_PILLAR=2,COL_SPAWNRAY=1,BASE_CENTER_X=320,BASE_CENTER_Y=200;
struct Runtime {
    CBossEntitySized<128,160> entity{};
    std::int16_t frame=0;
    struct {std::int16_t unused=0;} pattern;
    std::function<void(bool)> casting;
    bool meteor=true;
};
Runtime* active=nullptr;
Runtime& current() {if(!active) throw std::logic_error("Mima pillars are not bound");return *active;}
std::uint8_t pillar_color=2;
void grcg_setcolor_rmw(int color) {pillar_color=color;}
void grcg_off_func() {}
vram_offset_t vram_offset_shift(int x,int y) {return std::int16_t(y*80+(x>=0 ? x/8 : -((-x+7)/8)));}
void grcg_put(vram_offset_t offset,std::uint32_t mask,int) {
    auto& fb=th01_native::graphics_framebuffer();auto page=th01_native::current_render_page();
    for(unsigned bit=0;bit<32;++bit) if(mask&((0x80u>>(bit&7))<<((bit/8)*8))) {int linear=int(offset)*8+bit;if(linear>=0 && linear<640*400) fb.put(page,linear%640,linear/640,pillar_color);}
}
void select_for_rank(std::int16_t& value,int easy,int normal,int hard,int lunatic) {value=rank==RANK_EASY ? easy : rank==RANK_HARD ? hard : rank==RANK_LUNATIC ? lunatic : normal;}
void mima_put_cast_both() {current().meteor=false;current().casting(true);}
void meteor_activate() {if(!current().meteor) {current().meteor=true;current().casting(false);}}
#include "pillar_sprites.inc"
#define sPILLAR native_pillar_rows
#define ent_still current().entity
#define boss_phase_frame current().frame
#define pattern_state current().pattern
#define graph_r_line_unput native_graph_line_unput
#include "th01/main/boss/mima_pillars.inc"
#undef sPILLAR
#undef ent_still
#undef boss_phase_frame
#undef pattern_state
#undef graph_r_line_unput
}
namespace th01_native {
struct MimaPillarPattern::Impl {
    Runtime runtime;bool done=false;
    Impl(std::function<void(bool)> casting,screen_x_t left,screen_y_t top) {
        if(active) throw std::logic_error("Original Mima pillar pattern supports one active instance");
        if(!casting) throw std::invalid_argument("Mima casting renderer is required");
        runtime.casting=std::move(casting);runtime.entity.pos_cur_set(left,top);active=&runtime;
    }
    ~Impl() {active=nullptr;}
};
MimaPillarPattern::MimaPillarPattern(std::function<void(bool)> casting,screen_x_t left,screen_y_t top):impl(std::make_unique<Impl>(std::move(casting),left,top)) {}
MimaPillarPattern::~MimaPillarPattern()=default;
bool MimaPillarPattern::advance() {if(impl->done) return true;++impl->runtime.frame;pattern_pillars_and_aimed_spreads();impl->done=impl->runtime.frame==0;return impl->done;}
unsigned MimaPillarPattern::frame() const {return impl->runtime.frame;}
}