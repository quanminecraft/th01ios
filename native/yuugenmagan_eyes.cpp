#include "yuugenmagan_eyes.hpp"
#include "th01/main/boss/yuugenmagan_types.hpp"
#include "line_backend.hpp"
#include "boss_entity_backend.hpp"
#include "pellet_backend.hpp"
#include "missile_backend.hpp"
#include "th01/main/bullet/line.hpp"
#include "th01/math/dir.hpp"
#include "bomb_backend.hpp"
#include "hp_backend.hpp"
#include "th01/main/playfld.hpp"
namespace {
using dots8_t=std::uint8_t;
void graph_r_vline(screen_x_t x,screen_y_t top,screen_y_t bottom,vc2 color){graph_r_line(x,top,x,bottom,color);}
#define graph_r_line_unput native_graph_line_unput
#include "th01/main/boss/yuugenmagan_eye.inc"
struct Runtime {CEyeEntity eyes[5]{};Palette4 palette{};struct{bool initial_hp_rendered=false;} phase;struct{bool16 invincible=false;} hit;struct{int subphase=0,pairs=0,iterations=0;screen_x_t target=0;unsigned char west=0,east=0;bool clockwise=false;} missile;struct{int subphase=0;pixel_t x_edge_offset=0;unsigned char angle=0;} laser;struct{std::int16_t subphase=0,subphase_frame=0,iterations_done=0;unsigned char pellet_east=0;} spray;struct{int subphase=0;pixel_t distance=0;screen_x_t target_left=0;} shrink;struct{unsigned close=0,open=0,decrease=3,increase=3;yuugenmagan_int_t frame=0;} transition;};
Runtime* active=nullptr;
Runtime& current(){if(!active)throw std::logic_error("YuugenMagan eyes are not bound");return *active;}
#define eye_west current().eyes[0]
#define eye_east current().eyes[1]
#define eye_southwest current().eyes[2]
#define eye_southeast current().eyes[3]
#define eye_north current().eyes[4]
std::int16_t boss_hp=0,boss_phase_frame=0;std::int8_t boss_phase=0;
#include "th01/main/boss/yuugenmagan_eye_helpers.inc"
#include "th01/main/boss/yuugenmagan_setup.inc"
#include "th01/main/boss/yuugenmagan_rank.inc"
#include "th01/main/boss/yuugenmagan_intro_helpers.inc"
#include "th01/main/boss/yuugenmagan_lateral.inc"
struct point_t{pixel_t x,y;};
#include "th01/main/boss/yuugenmagan_missiles.inc"
#include "th01/main/boss/yuugenmagan_lasers.inc"
#define u1 current().laser
#define u2 current().laser
#define pentagram_ current().laser
void laser_tick(){
#include "th01/main/boss/yuugenmagan_laser_dispatch.inc"
}
#undef u1
#undef u2
#undef pentagram_
#define u1 current().spray
#define u2 current().spray
#define u3 current().spray
#define angle current().spray
void spray_tick(){
    enum {P5_PREPARE_1=0,P5_SPRAY_1=1,P5_PREPARE_2=2,P5_SPRAY_2=3};
#include "th01/main/boss/yuugenmagan_spray.inc"
}
#undef u1
#undef u2
#undef u3
#undef angle
using screen_point_t=th01_native::MimaPoint;
#define inhibit_Z3(x) (x)
#include "th01/main/boss/yuugenmagan_pentagram.inc"
#undef inhibit_Z3
#define u1 current().shrink
#define u2 current().shrink
#define target_left current().shrink.target_left
void shrink_tick(){
    enum {P11_PREPARE=0,P11_CORNERS_IN_EYES=1,P11_MOVE=2,P11_DONE=3};
#include "th01/main/boss/yuugenmagan_shrink.inc"
}
#undef u1
#undef u2
#undef target_left
Pentagram regular_attack{};
struct {int unused=0;bool handoff=false;} regular_vars;
struct RegularPhase {
    void back_from_13_to_10(pentagram_attack_phase_t next){regular_attack.phase=next;boss_phase_frame=0;boss_phase=10;regular_vars.unused=0;pentagram.unput();regular_vars.handoff=true;}
} regular_phase;
void regular_tick(){
    auto& pentagram_=regular_attack;auto& u2=regular_vars;auto& phase=regular_phase;
    enum {GROW_RADIUS=SUBPHASE_TIMEOUT_FRAMES/PENTAGRAM_INTERVAL,GROWN_RADIUS=PENTAGRAM_RADIUS_FINAL+GROW_RADIUS};
    struct {unsigned char tmp;} angle;
#include "th01/main/boss/yuugenmagan_regular.inc"
}
void boss_palette_snap(){current().palette=native_bomb_palette();}
#define z_Palettes native_bomb_palette()
#define stage_palette const_cast<Palette4&>(native_bomb_stage_palette())
#define phase current().phase
#define hit current().hit
void intro_tick(){
#include "th01/main/boss/yuugenmagan_intro_body.inc"
}
struct ComponentRef {
    std::int16_t& value;
    operator int()const{return value;}
    void change(int delta){unsigned byte=unsigned(int(value)+delta)&255u;value=byte<128 ? int(byte) : int(byte)-256;}
    ComponentRef& operator++(){change(1);return *this;}
    ComponentRef& operator--(){change(-1);return *this;}
    void operator++(int){change(1);}
    void operator--(int){change(-1);}
};
#define yuugenmagan_component(color,index) ComponentRef{(color).component(index)}
#include "th01/main/boss/yuugenmagan_transition.inc"
#undef yuugenmagan_component
#undef z_Palettes
#undef stage_palette
#undef phase
#undef hit
}
namespace th01_native {
struct YuugenmaganEyes::Impl {
    Runtime runtime;
    Impl(){if(active)throw std::logic_error("YuugenMagan eyes support one active instance");active=&runtime;try{yuugenmagan_ent_load();yuugenmagan_setup();}catch(...){yuugenmagan_ent_free();active=nullptr;throw;}}
    ~Impl(){yuugenmagan_ent_free();active=nullptr;}
};
YuugenmaganEyes::YuugenmaganEyes():impl(std::make_unique<Impl>()){}
YuugenmaganEyes::~YuugenmaganEyes()=default;
void YuugenmaganEyes::track_and_render(unsigned mask){if(mask>31)throw std::invalid_argument("Eye flags");eyes_locked_unput_and_put_then_track(static_cast<eye_flag_t>(mask));}
void YuugenmaganEyes::set_cel(unsigned mask,unsigned cel){if(mask>31 || cel>6)throw std::invalid_argument("Eye flags/cel");eyes_set_image(mask,cel);}
YuugenmaganEyeState YuugenmaganEyes::eye(unsigned index)const{if(index>=5)throw std::out_of_range("Eye index");auto& e=impl->runtime.eyes[index];return{e.cur_left,e.cur_top,e.image(),e.lock_frame,!e.hitbox_orb_inactive};}
bool YuugenmaganEyes::advance_intro(){if(boss_phase!=0)return true;intro_tick();return boss_phase!=0;}
int YuugenmaganEyes::intro_frame()const{return boss_phase_frame;}
void YuugenmaganEyes::advance_lateral(){if(boss_phase!=1)throw std::logic_error("Lateral attack requires completed intro");++boss_phase_frame;eyes_locked_unput_and_put_then_track(EF_WEST|EF_EAST);phase_1_pellets_from_lateral();}
void YuugenmaganEyes::begin_missiles(bool clockwise){boss_phase=clockwise ? 7 : 3;boss_phase_frame=0;impl->runtime.missile={};impl->runtime.missile.clockwise=clockwise;eyes_set_image((EF_SOUTHWEST|EF_SOUTHEAST),C_DOWN);Missiles.load(PTN_SLOT_BOSS_1);Missiles.reset();}
void YuugenmaganEyes::advance_missiles(){
    if(boss_phase!=3 && boss_phase!=7)throw std::logic_error("Missile pattern not selected");++boss_phase_frame;auto& m=impl->runtime.missile;
    if(m.clockwise){pattern_missile_pairs_from_south(m.subphase,m.pairs,m.target,m.west,m.east,m.iterations,2,missile_pairs_shift_angle_1_clock,missile_pairs_shift_angle_2_clock);}
    else{pattern_missile_pairs_from_south(m.subphase,m.pairs,m.target,m.west,m.east,m.iterations,3,missile_pairs_shift_angle_1_away,missile_pairs_shift_angle_2_towards);}
}
YuugenmaganMissileState YuugenmaganEyes::missile_state()const{auto& m=impl->runtime.missile;return{boss_phase_frame,m.subphase,m.pairs,m.iterations,m.target,m.west,m.east};}
void YuugenmaganEyes::begin_lasers(){boss_phase=9;boss_phase_frame=0;impl->runtime.laser={};eye_north.set_image(C_DOWN);}
bool YuugenmaganEyes::advance_lasers(){if(boss_phase!=9)throw std::logic_error("Laser pattern not selected");if(impl->runtime.laser.subphase==6)return true;++boss_phase_frame;laser_tick();return impl->runtime.laser.subphase==6;}
void YuugenmaganEyes::begin_spray(){boss_phase=5;boss_phase_frame=0;impl->runtime.spray={};pattern_interval=select_for_rank(12,8,4,2);eyes_set_image((EF_WEST|EF_EAST),C_DOWN);}
void YuugenmaganEyes::advance_spray(){if(boss_phase!=5)throw std::logic_error("Spray pattern not selected");++boss_phase_frame;spray_tick();}
YuugenmaganSprayState YuugenmaganEyes::spray_state()const{auto& s=impl->runtime.spray;return{boss_phase_frame,s.subphase,s.subphase_frame,s.iterations_done,s.pellet_east};}
void YuugenmaganEyes::begin_pentagram_shrink(){boss_phase=11;boss_phase_frame=0;impl->runtime.shrink={};pentagram={};eyes_set_image(31,C_DOWN);}
bool YuugenmaganEyes::advance_pentagram_shrink(){if(boss_phase!=11)throw std::logic_error("Pentagram shrink not selected");if(impl->runtime.shrink.subphase==3)return true;++boss_phase_frame;shrink_tick();return impl->runtime.shrink.subphase==3;}
YuugenmaganPentagramState YuugenmaganEyes::pentagram_state()const{auto& s=impl->runtime.shrink;YuugenmaganPentagramState result{boss_phase_frame,s.subphase,s.distance,{}};const eye_flag_t flags[]={EF_NORTH,EF_SOUTHWEST,EF_EAST,EF_WEST,EF_SOUTHEAST};for(unsigned i=0;i<5;++i)result.points[i]={pentagram_shrink_x(flags[i],s.distance),pentagram_shrink_y(flags[i],s.distance)};return result;}
void YuugenmaganEyes::begin_regular_pentagram(bool grow){boss_phase=13;boss_phase_frame=0;regular_vars={};regular_attack={};regular_attack.phase=grow ? PAP_PREPARE_2 : PAP_PREPARE_1;pentagram={};pentagram.center={316,PENTAGRAM_REGULAR_CENTER_Y};pentagram.radius=PENTAGRAM_RADIUS_FINAL;pentagram_corners_set_regular(PENTAGRAM_ANGLE_INITIAL);pattern_interval=select_for_rank(24,14,10,8);}
bool YuugenmaganEyes::advance_regular_pentagram(){if(regular_vars.handoff)return true;if(boss_phase!=13)throw std::logic_error("Regular pentagram not selected");++boss_phase_frame;regular_tick();return regular_vars.handoff;}
YuugenmaganRegularState YuugenmaganEyes::regular_state()const{YuugenmaganRegularState out{boss_phase_frame,int(regular_attack.phase),regular_attack.angle,pentagram.center.x,pentagram.center.y,pentagram.radius,regular_vars.handoff,{}};for(unsigned i=0;i<5;++i)out.points[i]={pentagram.x[i],pentagram.y[i]};return out;}
void YuugenmaganEyes::begin_transition(unsigned close,unsigned open,unsigned decrease,unsigned increase){if(close>31 || open>31 || decrease>3 || increase>3)throw std::invalid_argument("Eye transition flags/components");impl->runtime.transition={close,open,decrease,increase,0};}
bool YuugenmaganEyes::advance_transition(){auto& t=impl->runtime.transition;if(t.frame>=EYE_TOGGLE_FRAMES)return true;eyes_toggle_and_yokoshima_recolor(static_cast<eye_flag_t>(t.close),static_cast<eye_flag_t>(t.open),t.decrease,t.increase,t.frame);return t.frame>=EYE_TOGGLE_FRAMES;}
int YuugenmaganEyes::transition_frame()const{return impl->runtime.transition.frame;}
YuugenmaganLaserState YuugenmaganEyes::laser_state()const{auto& l=impl->runtime.laser;return{boss_phase_frame,l.subphase,l.x_edge_offset};}
}
