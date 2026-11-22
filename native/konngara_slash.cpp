#include "konngara_slash.hpp"
#include "pellet_backend.hpp"
namespace {
struct Runtime {
    std::int16_t frame=0;bool16 direction_allowed=true;
    union {std::int16_t interval,unused;subpixel_t speed;} pattern{};
    th01_native::KonngaraSlashHost host;
};
Runtime* active=nullptr;
Runtime& current() {if(!active) throw std::logic_error("Konngara slash pattern is not bound");return *active;}
constexpr int FD_CENTER=2,FE_CLOSED=1,FE_AIM=3;
constexpr screen_x_t SWORD_CENTER_X=410,SLASH_4_CORNER_X=432,SLASH_5_CORNER_X=198;
constexpr screen_y_t SWORD_CENTER_Y=70,SLASH_4_CORNER_Y=232,SLASH_5_CORNER_Y=198;
constexpr pixel_t SLASH_DISTANCE_2_TO_4_X=22,SLASH_DISTANCE_2_TO_4_Y=162;
constexpr pixel_t SLASH_DISTANCE_5_TO_4_X=-234,SLASH_DISTANCE_5_TO_4_Y=-34;
constexpr pixel_t SLASH_DISTANCE_5_TO_6_X=-212,SLASH_DISTANCE_5_TO_6_Y=128;
void face_direction_set_and_put(int) {current().host.face_center();}
void face_expression_set_and_put(int value) {current().host.expression(static_cast<th01_native::KonngaraExpression>(value));}
void slash_put(unsigned image) {current().host.slash(image);}
void select_for_rank(std::int16_t& value,int easy,int normal,int hard,int lunatic) {value=rank==RANK_EASY ? easy : rank==RANK_HARD ? hard : rank==RANK_LUNATIC ? lunatic : normal;}
void select_subpixel_for_rank(subpixel_t& value,float easy,float normal,float hard,float lunatic) {value=to_sp(rank==RANK_EASY ? easy : rank==RANK_HARD ? hard : rank==RANK_LUNATIC ? lunatic : normal);}
#define boss_phase_frame current().frame
#define pattern_state current().pattern
#define face_direction_can_change current().direction_allowed
#include "th01/main/boss/konngara_slash.inc"
#include "th01/main/boss/konngara_slash_aimed.inc"
#undef boss_phase_frame
#undef pattern_state
#undef face_direction_can_change
}
namespace th01_native {
struct KonngaraSlashPattern::Impl {
    Runtime runtime;KonngaraSlashAttack attack;bool done=false;
    Impl(KonngaraSlashAttack value,KonngaraSlashHost host):attack(value) {
        if(active) throw std::logic_error("Original Konngara slash patterns support one active instance");
        if(!host.expression || !host.face_center || !host.slash) throw std::invalid_argument("Konngara slash host callbacks required");
        if(value!=KonngaraSlashAttack::rain && value!=KonngaraSlashAttack::triangular && value!=KonngaraSlashAttack::aimed) throw std::invalid_argument("Konngara slash attack");
        runtime.host=std::move(host);active=&runtime;
    }
    ~Impl() {active=nullptr;}
};
KonngaraSlashPattern::KonngaraSlashPattern(KonngaraSlashAttack attack,KonngaraSlashHost host):impl(std::make_unique<Impl>(attack,std::move(host))) {}
KonngaraSlashPattern::~KonngaraSlashPattern()=default;
bool KonngaraSlashPattern::advance() {
    if(impl->done) return true;++impl->runtime.frame;
    if(impl->attack==KonngaraSlashAttack::rain) pattern_slash_rain();
    else if(impl->attack==KonngaraSlashAttack::triangular) pattern_slash_triangular();
    else pattern_slash_aimed();
    impl->done=impl->runtime.frame==0;return impl->done;
}
unsigned KonngaraSlashPattern::frame() const {return impl->runtime.frame;}
bool KonngaraSlashPattern::face_direction_change_allowed() const {return impl->runtime.direction_allowed;}
}
