#include "laser_backend.hpp"
#include "elis_teleport.hpp"
#include "line_backend.hpp"
#include <chrono>
#include <thread>
#include "boss_entity_backend.hpp"
#include "missile_backend.hpp"
#include "th01/math/dir.hpp"
#include "hp_backend.hpp"
#include "bomb_backend.hpp"
#include "th01/main/shape.hpp"
#include "th01/main/entity.hpp"
#include "th01/main/playfld.hpp"
namespace {
using vc_t=std::uint8_t;
#include "th01/main/boss/elis_entities.inc"
struct Runtime{std::array<std::vector<std::uint8_t>,12> tiles;int target_left=0,target_top=0;std::uint32_t seed=1;bool done=true,transforming=false,to_bat=false,flying=false,starring=false,final_active=false;int final_subphase=0,final_bat=0,final_girl=0;unsigned phase3_attack=0,human_pattern=0,flight_pattern=0;pixel_t vx=0,vy=0;elis_form_t form=F_GIRL;th01_native::SpriteSet grc;std::function<void(unsigned)> wait,delay;};
Runtime* active=nullptr;
Runtime& current(){if(!active)throw std::logic_error("Elis teleport is not bound");return *active;}
std::int16_t boss_hp=0,boss_phase_frame=0;std::int8_t boss_phase=0;
#define random_seed native_random_seed
#define frame_rand current().seed
#include "th01/main/boss/elis_setup.inc"
#include "th01/main/boss/elis_surround.inc"
#include "th01/main/boss/elis_form_coords.inc"
#include "th01/main/boss/elis_phase5_enum.inc"
#include "th01/main/boss/elis_bat_flight.inc"
void select_for_rank(int& ret,int easy,int normal,int hard,int lunatic){ret=rank==RANK_EASY ? easy:rank==RANK_HARD ? hard:rank==RANK_LUNATIC ? lunatic:normal;}
#include "th01/main/boss/elis_bat_attacks.inc"
void select_for_rank(std::int16_t& ret,int easy,int normal,int hard,int lunatic){int value;select_for_rank(value,easy,normal,hard,lunatic);ret=value;}
void select_laser_speed_for_rank(int& ret,float easy,float normal,float hard,float lunatic){ret=to_sp((rank==RANK_EASY ? easy:rank==RANK_HARD ? hard:rank==RANK_LUNATIC ? lunatic:normal)/2);}
void select_for_rank(pellet_group_t& ret,int easy,int normal,int hard,int lunatic){int value;select_for_rank(value,easy,normal,hard,lunatic);ret=static_cast<pellet_group_t>(value);}
constexpr int V_WHITE=7;
constexpr int CHOOSE_NEW=0;
#include "th01/main/boss/elis_star_enum.inc"
#include "th01/main/boss/elis_pattern_state.inc"
void bat_attack(unsigned pattern){switch(pattern){case 0:break;case 1:pattern_bat_slow_spreads();break;case 2:pattern_bat_alternating_narrow_and_wide_2_spreads();break;case 3:pattern_bat_random_rain();break;case 4:pattern_bat_alternating_2_and_3_spreads();break;default:throw std::invalid_argument("Elis bat pattern");}}
int floor8(int x){return x>=0 ? x/8*8 : -((-x+7)/8)*8;}
void ptn_snap_rect_from_1_8(int left,int top,int w,int h,int slot,int& image,int& x,int& y){
    if(slot!=PTN_SLOT_BOSS_1)throw std::invalid_argument("Elis background slot");auto& fb=th01_native::graphics_framebuffer();
    for(y=0;y<h/32;++y)for(x=0;x<w/32;++x){auto& tile=current().tiles.at(image++);tile.resize(1024);for(int py=0;py<32;++py)for(int px=0;px<32;++px){int sx=floor8(left)+x*32+px,sy=top+y*32+py;tile[py*32+px]=(sx>=0 && sx<640 && sy>=0 && sy<400) ? fb.pixel(1,sx,sy):0;}}
    page_access(0);
}
void ptn_put_rect_noalpha_8(int left,int top,int w,int h,int slot,int& image,int& x,int& y){
    if(slot!=PTN_SLOT_BOSS_1)throw std::invalid_argument("Elis background slot");auto& fb=th01_native::graphics_framebuffer();auto page=th01_native::current_render_page();
    for(y=0;y<h/32;++y)for(x=0;x<w/32;++x){auto& tile=current().tiles.at(image++);for(int py=0;py<32;++py)for(int px=0;px<32;++px){int dx=floor8(left)+x*32+px,dy=top+y*32+py;if(dx>=0 && dx<640 && dy>=0 && dy<400)fb.put(page,dx,dy,tile.at(py*32+px));}}
}
#include "th01/main/boss/elis_background.inc"
#include "th01/main/boss/elis_symmetric.inc"
#include "th01/main/boss/elis_curved_rings.inc"
#include "th01/main/boss/elis_lasers.inc"
#include "th01/main/boss/elis_bigcircle.inc"
#include "th01/main/boss/elis_circle_pellets.inc"
#define graph_r_line_unput native_graph_line_unput
#include "th01/main/boss/elis_star_lines.inc"
#include "th01/main/boss/elis_star.inc"
void delay(unsigned ms){if(current().delay)current().delay(ms);else std::this_thread::sleep_for(std::chrono::milliseconds(ms));}
#include "th01/main/boss/elis_safety_circle.inc"
#include "th01/main/boss/elis_final_lasers.inc"
#include "th01/main/boss/elis_teleport.inc"
// Elis owns its boss GRC bank independently of bomb glyph slots.
void native_elis_grc_put(screen_x_t left,screen_y_t top,int slot,int image,unsigned color){
    if(slot!=GRC_SLOT_BOSS_1)throw std::invalid_argument("Elis GRC slot");
    auto& sprite=current().grc.images.at(image);auto& fb=th01_native::graphics_framebuffer();
    if(left< -sprite.width || left>=640 || top< -sprite.height || top>=400)return;
    int row=top*80+floor8(left)/8;
    for(int y=0;y<sprite.height;++y){
        if(row>=0)for(int byte=0;byte<sprite.width/8;++byte){int off=row+byte;if(!((left>=0 && off/80==row/80)||(left<0 && off/80!=row/80)))continue;
            for(int bit=0;bit<8;++bit)if(sprite.opaque.at(y*sprite.width+byte*8+bit)){int linear=off*8+bit;if(linear>=0 && linear<640*400)fb.put(th01_native::current_render_page(),linear%640,linear/640,color);}}
        row+=80;if(row>=32000)break;
    }
}
#define grc_put_8 native_elis_grc_put
#undef grc_sloppy_unput
#define grc_sloppy_unput(left,top) egc_copy_rect_1_to_0_16(left,top,48,32);
#include "th01/main/boss/elis_grc.inc"
void frame_delay(unsigned frames){current().wait(frames);}
void native_elis_scroll(screen_y_t y){current().wait(1);th01_native::graphics_framebuffer().display_scroll_y=(y%400+400)%400;}
#define z_vsync_wait_and_scrollup native_elis_scroll
#include "th01/main/boss/elis_rifts.inc"
#include "th01/main/boss/elis_rift_missiles.inc"
#include "th01/main/boss/elis_sphere_clusters.inc"
#include "th01/main/boss/elis_rift_pellets.inc"
#include "th01/main/boss/elis_phase3.inc"
#include "th01/main/boss/elis_transform.inc"
void native_elis_observe_girl(int pattern){current().final_girl=pattern;}
void native_elis_observe_final(int subphase,int pattern){current().final_subphase=subphase;current().final_bat=pattern;}
#include "th01/main/boss/elis_phase5.inc"
void entrance_body(){
    struct {int invincibility_frame=0;} hit;
    struct {bool teleport_done=false;struct {int pattern=0;}cur;} phase;
    bool initial_hp_rendered=false;screen_x_t head_left;screen_y_t head_top;bool16 trails_offscreen;unsigned char angle;
#include "th01/main/boss/elis_entrance.inc"
}
}
namespace th01_native {
struct ElisTeleport::Impl{
    Runtime runtime;
    Impl(std::uint32_t seed){if(active)throw std::logic_error("Elis supports one active teleport controller");active=&runtime;native_elis_girl={};native_elis_attack={};native_elis_bat={};runtime.seed=seed;try{elis_ent_load();elis_setup();girl_bg_snap(1);ent_put_both(ent_still_or_wave,C_STILL);}catch(...){elis_ent_free();active=nullptr;throw;}}
    ~Impl(){elis_ent_free();active=nullptr;}
};
ElisTeleport::ElisTeleport(std::uint32_t seed):impl(std::make_unique<Impl>(seed)){}
ElisTeleport::~ElisTeleport()=default;
void ElisTeleport::entrance(const std::vector<Entry>& archive,std::function<void(unsigned)> wait){
    if(!wait)throw std::invalid_argument("Elis entrance wait callback required");auto& r=impl->runtime;
    if(!r.done)throw std::logic_error("Cannot enter during teleport");
    r.grc=read_sprites(asset(archive,"BOSS5_GR.GRC"));r.wait=std::move(wait);
    page_access(1);girl_bg_put(1);page_access(0);girl_bg_put(1);entrance_body();
}
void ElisTeleport::begin_final_phase(){auto& r=impl->runtime;if(r.form!=F_BAT || r.transforming || r.flying || r.human_pattern || r.starring || r.phase3_attack || r.final_active)throw std::logic_error("Elis final phase needs idle bat");boss_phase_frame=0;r.vx=r.vy=0;r.final_bat=r.final_girl=r.final_subphase=0;phase_5(r.form,r.vx,r.vy,true);r.final_active=true;}
void ElisTeleport::advance_final_phase(){auto& r=impl->runtime;if(!r.final_active)throw std::logic_error("Elis final phase not started");++boss_phase_frame;auto form=r.form;auto bat_velocity_x=r.vx,bat_velocity_y=r.vy;
#include "th01/main/boss/elis_bat_move.inc"
    phase_5(r.form,r.vx,r.vy);
}
ElisFinalState ElisTeleport::final_state()const{auto& r=impl->runtime;return{r.form==F_BAT,boss_phase_frame,r.final_subphase,r.final_bat,r.final_girl};}
void ElisTeleport::begin_phase3(unsigned attack){auto& r=impl->runtime;if(attack<1 || attack>3)throw std::invalid_argument("Elis phase-three attack");if(r.form!=F_GIRL || !r.done || r.transforming || r.flying || r.human_pattern || r.starring || r.phase3_attack)throw std::logic_error("Elis phase three needs idle girl");if(attack>1 && r.grc.images.empty())throw std::logic_error("Elis phase-three effects need entrance");phase_3(99);r.phase3_attack=attack;boss_phase_frame=0;page_access(0);}
bool ElisTeleport::advance_phase3(){auto& r=impl->runtime;if(!r.phase3_attack)return true;++boss_phase_frame;r.phase3_attack=phase_3(r.phase3_attack);return r.phase3_attack==0;}
void ElisTeleport::begin_star(){auto& r=impl->runtime;if(r.form!=F_GIRL || !r.done || r.transforming || r.flying || r.human_pattern || r.starring || r.phase3_attack)throw std::logic_error("Elis star needs idle girl");r.starring=true;boss_phase_frame=0;page_access(0);}
bool ElisTeleport::advance_star(){auto& r=impl->runtime;if(!r.starring)return true;++boss_phase_frame;r.starring=star_of_david()!=SP_PATTERN;return !r.starring;}
void ElisTeleport::set_delay(std::function<void(unsigned)> fn){impl->runtime.delay=std::move(fn);}
void ElisTeleport::begin_human_pattern(unsigned pattern){auto& r=impl->runtime;if(pattern<1 || pattern>9)throw std::invalid_argument("Elis human pattern");if(r.form!=F_GIRL || !r.done || r.transforming || r.flying || r.human_pattern || r.starring || r.phase3_attack)throw std::logic_error("Elis human pattern needs idle girl");if((pattern==3 || pattern==6 || pattern==7) && r.grc.images.empty())throw std::logic_error("Elis rift effects require entrance first");r.human_pattern=pattern;boss_phase_frame=0;pattern_state={};page_access(0);}
bool ElisTeleport::advance_human_pattern(){
    auto& r=impl->runtime;if(!r.human_pattern)return true;++boss_phase_frame;bool done=false;
    switch(r.human_pattern){
    case 1:done=pattern_three_symmetric_4_stacks_then_symmetric_arc()==SP_STAR_OF_DAVID;break;
    case 2:done=pattern_curved_5_stack_rings()==CHOOSE_NEW;break;
    case 3:done=pattern_random_downwards_missiles()==CHOOSE_NEW;break;
    case 4:done=pattern_11_lasers_across()==CHOOSE_NEW;break;
    case 5:done=pattern_pellets_along_circle()==CHOOSE_NEW;break;
    case 6:done=pattern_clusters_from_spheres()==CHOOSE_NEW;break;
    case 7:done=pattern_random_from_rifts()==CHOOSE_NEW;break;
    case 8:done=pattern_safety_circle_and_rain_from_top()==SP_STAR_OF_DAVID;break;
    case 9:done=pattern_aimed_5_spreads_and_lasers_followed_by_ring()==SP_STAR_OF_DAVID;break;
    }
    if(done)r.human_pattern=0;return done;
}
void ElisTeleport::fire_bat_pattern(unsigned pattern,unsigned tick){auto& r=impl->runtime;if(r.form!=F_BAT || r.transforming || r.flying)throw std::logic_error("Elis pattern needs idle bat form");if(pattern<1 || pattern>4 || tick>32767)throw std::invalid_argument("Elis pattern/tick");boss_phase_frame=tick;bat_attack(pattern);}
void ElisTeleport::begin_flight(unsigned pattern){if(pattern>4)throw std::invalid_argument("Elis flight pattern");auto& r=impl->runtime;if(r.form!=F_BAT || r.transforming || r.flying)throw std::logic_error("Elis flight needs idle bat form");boss_phase_frame=0;r.vx=r.vy=0;r.flight_pattern=pattern;r.flying=true;}
bool ElisTeleport::advance_flight(){auto& r=impl->runtime;if(!r.flying)return true;++boss_phase_frame;auto form=r.form;auto bat_velocity_x=r.vx,bat_velocity_y=r.vy;
#include "th01/main/boss/elis_bat_move.inc"
    r.flying=bat_fly_random(r.vx,r.vy)!=P5_TRANSFORM;bat_attack(r.flight_pattern);return !r.flying;
}
void ElisTeleport::begin_transform(bool to_bat){
    auto& r=impl->runtime;if(!r.done || r.transforming || r.flying || r.human_pattern || r.starring || r.phase3_attack)throw std::logic_error("Elis action already running");
    if(r.grc.images.empty() || !r.wait)throw std::logic_error("Elis entrance must load effects first");
    if(to_bat==(r.form==F_BAT))throw std::logic_error("Elis is already in requested form");
    boss_phase_frame=0;r.to_bat=to_bat;r.transforming=true;
}
bool ElisTeleport::advance_transform(){auto& r=impl->runtime;if(!r.transforming)return true;++boss_phase_frame;r.form=r.to_bat ? transform_girl_to_bat():transform_bat_to_girl();r.transforming=r.form!=(r.to_bat ? F_BAT:F_GIRL);return !r.transforming;}
void ElisTeleport::begin(int left,int top){if(left<PLAYFIELD_LEFT || left>PLAYFIELD_RIGHT-GIRL_W || top<PLAYFIELD_TOP || top>PLAYFIELD_BOTTOM-GIRL_H)throw std::out_of_range("Elis teleport target outside playfield");if(!impl->runtime.done || impl->runtime.transforming || impl->runtime.form!=F_GIRL)throw std::logic_error("Elis cannot teleport in this state");impl->runtime.target_left=left;impl->runtime.target_top=top;boss_phase_frame=0;impl->runtime.done=false;}
bool ElisTeleport::advance(){auto& r=impl->runtime;if(r.done)return true;++boss_phase_frame;r.done=wave_teleport(r.target_left,r.target_top);return r.done;}
int ElisTeleport::frame()const{return boss_phase_frame;}
ElisEntityState ElisTeleport::girl()const{auto& e=native_elis_girl;return{e.cur_left,e.cur_top,e.image(),!e.hitbox_orb_inactive};}
ElisEntityState ElisTeleport::bat()const{auto& e=native_elis_bat;return{e.cur_left,e.cur_top,e.image(),!e.hitbox_orb_inactive};}
}