#include "konngara_early.hpp"
#include "pellet_backend.hpp"
#include "th01/main/playfld.hpp"
#include "th01/main/entity.hpp"
#include "th01/main/shape.hpp"
#include "th01/sprites/shape8x8.hpp"
#include "line_backend.hpp"
#include "laser_backend.hpp"
#include "th01/main/spawnray.hpp"
namespace {
constexpr int RES_Y=400;
struct point_t {pixel_t x,y;};
struct Runtime {
    std::int16_t frame=0;
    screen_x_t ray_prev_x=-PIXEL_NONE;screen_y_t ray_prev_y=-PIXEL_NONE;
    union {std::int16_t group,interval,unused;subpixel_t speed;pixel_t delta_x;} pattern{};
    std::function<void(th01_native::KonngaraExpression)> expression;
};
Runtime* active=nullptr;
Runtime& current() {if(!active) throw std::logic_error("Konngara early pattern is not bound");return *active;}
constexpr int FE_NEUTRAL=0,FE_CLOSED=1,FE_GLARE=2,FE_AIM=3;
constexpr screen_x_t CUP_LEFT=296,CUP_RIGHT=328,CUP_CENTER_X=312,SWORD_CENTER_X=410,EYE_CENTER_X=316,LEFT_SLEEVE_LEFT=290;
constexpr screen_y_t CUP_TOP=188,SWORD_CENTER_Y=70,EYE_BOTTOM=140,LEFT_SLEEVE_TOP=200;
void face_expression_set_and_put(int value) {current().expression(static_cast<th01_native::KonngaraExpression>(value));}
void select_for_rank(std::int16_t& value,int easy,int normal,int hard,int lunatic) {value=rank==RANK_EASY ? easy : rank==RANK_HARD ? hard : rank==RANK_LUNATIC ? lunatic : normal;}
void select_subpixel_for_rank(subpixel_t& value,float easy,float normal,float hard,float lunatic) {value=to_sp(rank==RANK_EASY ? easy : rank==RANK_HARD ? hard : rank==RANK_LUNATIC ? lunatic : normal);}
#define graph_r_line_unput native_graph_line_unput
void spawnray_unput_and_put(screen_x_t origin_x,screen_y_t origin_y,screen_x_t target_x,screen_y_t target_y,vc2 col) {
    spawnray_unput_and_put_func(current().ray_prev_x,current().ray_prev_y,origin_x,origin_y,target_x,target_y,col,false);
}
#define boss_phase_frame current().frame
#define pattern_state current().pattern
#define far
#include "th01/main/boss/konngara_snakes_common.inc"
#include "th01/main/boss/konngara_early.inc"
#include "th01/main/boss/konngara_two_snakes.inc"
#include "th01/main/boss/konngara_four_snakes.inc"
#include "th01/main/boss/konngara_rows_spray.inc"
#include "th01/main/boss/konngara_swordray.inc"
#include "th01/main/boss/konngara_lasers.inc"
#include "th01/main/boss/konngara_sleeve.inc"
#undef graph_r_line_unput
#undef far
#undef boss_phase_frame
#undef pattern_state
}
namespace th01_native {
struct KonngaraEarlyPattern::Impl {
    Runtime runtime;KonngaraEarlyAttack attack;bool done=false;
    Impl(KonngaraEarlyAttack value,std::function<void(KonngaraExpression)> expression):attack(value) {
        if(active) throw std::logic_error("Original Konngara early patterns support one active instance");
        if(!expression) throw std::invalid_argument("Konngara expression renderer required");
        if(value!=KonngaraEarlyAttack::diamond_cross_rain && value!=KonngaraEarlyAttack::cup_spreads && value!=KonngaraEarlyAttack::two_snakes_spreads && value!=KonngaraEarlyAttack::four_snakes && value!=KonngaraEarlyAttack::aimed_rows && value!=KonngaraEarlyAttack::cup_spray && value!=KonngaraEarlyAttack::rain_edges && value!=KonngaraEarlyAttack::lasers_spread && value!=KonngaraEarlyAttack::sleeve_rain) throw std::invalid_argument("Konngara early attack");
        runtime.expression=std::move(expression);active=&runtime;
    }
    ~Impl() {active=nullptr;}
};
KonngaraEarlyPattern::KonngaraEarlyPattern(KonngaraEarlyAttack attack,std::function<void(KonngaraExpression)> expression):impl(std::make_unique<Impl>(attack,std::move(expression))) {}
KonngaraEarlyPattern::~KonngaraEarlyPattern()=default;
bool KonngaraEarlyPattern::advance() {
    if(impl->done) return true;
    ++impl->runtime.frame;
    if(impl->attack==KonngaraEarlyAttack::diamond_cross_rain) pattern_diamond_cross_to_edges_followed_by_rain();
    else if(impl->attack==KonngaraEarlyAttack::cup_spreads) pattern_symmetrical_from_cup();
    else if(impl->attack==KonngaraEarlyAttack::two_snakes_spreads) pattern_two_homing_snakes_and_semicircle_spreads();
    else if(impl->attack==KonngaraEarlyAttack::four_snakes) pattern_four_homing_snakes();
    else if(impl->attack==KonngaraEarlyAttack::aimed_rows) pattern_aimed_rows_from_top();
    else if(impl->attack==KonngaraEarlyAttack::cup_spray) pattern_aimed_spray_from_cup();
    else if(impl->attack==KonngaraEarlyAttack::rain_edges) pattern_rain_from_edges();
    else if(impl->attack==KonngaraEarlyAttack::lasers_spread) pattern_lasers_and_3_spread();
    else pattern_semicircle_rain_from_sleeve();
    impl->done=impl->runtime.frame==0;return impl->done;
}
unsigned KonngaraEarlyPattern::frame() const {return impl->runtime.frame;}
}
