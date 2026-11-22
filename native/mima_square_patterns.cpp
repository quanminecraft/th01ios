#include "mima_square_patterns.hpp"
#include "line_backend.hpp"
#include <algorithm>
#include "boss_entity_backend.hpp"
#include "pellet_backend.hpp"
#include "th01/main/bullet/missile.hpp"
#include "th01/main/bullet/laser_s.hpp"
namespace {
using screen_point_t=th01_native::MimaPoint;
struct Runtime {
    CBossEntitySized<128,160> entity{};
    std::int16_t frame=0;
    union {subpixel_t speed;DecimalSubpixel speed_decimal;std::int16_t speed_multiplied_by_8;} pattern{};
    th01_native::MimaSquareLines lines;
    std::function<void(bool)> casting;bool meteor=true;
};
Runtime* active=nullptr;
Runtime& current() {if(!active) throw std::logic_error("Mima pattern is not bound");return *active;}
constexpr pixel_t SEAL_RADIUS=80,SEAL_CIRCUMSQUARE_RADIUS=112;
#define ent_still current().entity
#define boss_phase_frame current().frame
#define pattern_state current().pattern
void select_subpixel_for_rank(subpixel_t& out,float easy,float normal,float hard,float lunatic) {out=to_sp(rank==RANK_EASY ? easy : rank==RANK_HARD ? hard : rank==RANK_LUNATIC ? lunatic : normal);}
void select_for_rank(std::int16_t& out,int easy,int normal,int hard,int lunatic) {out=rank==RANK_EASY ? easy : rank==RANK_HARD ? hard : rank==RANK_LUNATIC ? lunatic : normal;}
void select_laser_speed_for_rank(std::int16_t& out,float easy,float normal,float hard,float lunatic) {select_subpixel_for_rank(out,easy/2,normal/2,hard/2,lunatic/2);}
void graph_r_lineloop_unput(const screen_point_t* point,int count) {if(count!=4) throw std::logic_error("Square point count");std::array<screen_point_t,4> copy;std::copy(point,point+4,copy.begin());current().lines(true,copy);}
void graph_r_lineloop_put(const screen_point_t* point,int count,int) {if(count!=4) throw std::logic_error("Square point count");std::array<screen_point_t,4> copy;std::copy(point,point+4,copy.begin());current().lines(false,copy);}
void mima_put_cast_both() {current().meteor=false;current().casting(true);}
void meteor_activate() {if(!current().meteor) {current().meteor=true;current().casting(false);}}
#include "th01/main/boss/mima_square_patterns.inc"
#include "th01/main/boss/mima_square_late.inc"
#undef ent_still
#undef boss_phase_frame
#undef pattern_state
}
namespace th01_native {
struct MimaSquarePattern::Impl {
    Runtime runtime;
    MimaSquareAttack attack;
    bool done=false;
    Impl(MimaSquareAttack value,screen_x_t left,screen_y_t top,MimaSquareLines lines,std::function<void(bool)> casting):attack(value) {
        if(active) throw std::logic_error("Original Mima patterns support one active instance");
        if(!lines) lines=th01_native::render_mima_square;
        if((value==MimaSquareAttack::two_squares || value==MimaSquareAttack::lasers) && !casting) throw std::invalid_argument("Mima casting renderer is required");
        runtime.casting=std::move(casting);runtime.entity.pos_cur_set(left,top);runtime.lines=std::move(lines);sq={};active=&runtime;
    }
    ~Impl() {active=nullptr;}
};
MimaSquarePattern::MimaSquarePattern(MimaSquareAttack attack,screen_x_t left,screen_y_t top,MimaSquareLines lines,std::function<void(bool)> casting):impl(std::make_unique<Impl>(attack,left,top,std::move(lines),std::move(casting))) {}
MimaSquarePattern::~MimaSquarePattern()=default;
bool MimaSquarePattern::advance() {
    if(impl->done) return true;
    ++impl->runtime.frame;
    if(impl->attack==MimaSquareAttack::pellets) pattern_aimed_then_static_pellets_from_square_corners();
    else if(impl->attack==MimaSquareAttack::missiles) pattern_aimed_missiles_from_square_corners();
    else if(impl->attack==MimaSquareAttack::two_squares) pattern_static_pellets_from_corners_of_two_squares();
    else if(impl->attack==MimaSquareAttack::halfcircle_missiles) pattern_halfcircle_missiles_downwards_from_corners();
    else if(impl->attack==MimaSquareAttack::slow_spray) pattern_slow_pellet_spray_from_corners();
    else pattern_aimed_lasers_from_corners();
    impl->done=impl->runtime.frame==0;return impl->done;
}
unsigned MimaSquarePattern::frame() const {return impl->runtime.frame;}
}