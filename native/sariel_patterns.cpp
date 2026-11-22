#include "sariel_patterns.hpp"
#include "sariel_body.hpp"
#include "th01/sprites/shape8x8.hpp"
#include "laser_backend.hpp"
#include "line_backend.hpp"
#include "th01/main/spawnray.hpp"
#include "th01/math/dir.hpp"
#include "pellet_backend.hpp"
#include "th01/main/playfld.hpp"
#include "th01/main/shape.hpp"
namespace {
using sariel_frame_t=int;
constexpr int RES_Y=400;
bool active=false;
int boss_phase_frame=0;
union {int speed_multiplied_by_8;int interval;int start_frame;pixel_t velocity_x;} pattern_state{};
#include "th01/main/boss/sariel_coords.inc"
void select_for_rank(int& value,int easy,int normal,int hard,int lunatic){value=rank==RANK_EASY ? easy:rank==RANK_HARD ? hard:rank==RANK_LUNATIC ? lunatic:normal;}
void select_for_rank(pixel_t& value,int easy,int normal,int hard,int lunatic){int selected;select_for_rank(selected,easy,normal,hard,lunatic);value=selected;}
constexpr int COL_LASER=4,COL_DETONATION=5,COL_DETONATION_START=6;
th01_native::SarielBody* active_body=nullptr;
bool wand_render_raise_both(){return active_body->raise_wand();}
void wand_lower_both(){active_body->lower_wand();}
void select_laser_speed_for_rank(int& value,float easy,float normal,float hard,float lunatic){value=to_sp((rank==RANK_EASY ? easy:rank==RANK_HARD ? hard:rank==RANK_LUNATIC ? lunatic:normal)/2);}
#include "th01/main/boss/sariel_random_lasers.inc"
#include "th01/main/boss/sariel_orb_rings.inc"
screen_x_t polar_x(int center,int radius,unsigned char angle){return ::polar_x(pixel_t(center),pixel_t(radius),angle);}
screen_y_t polar_y(int center,int radius,unsigned char angle){return ::polar_y(pixel_t(center),pixel_t(radius),angle);}
#include "th01/main/boss/sariel_sling_clusters.inc"
#include "th01/main/boss/sariel_semicircles.inc"
#include "th01/main/boss/sariel_snowflake.inc"
#define graph_r_line_unput native_graph_line_unput
#include "th01/main/boss/sariel_ray.inc"
#include "th01/main/boss/sariel_four_lasers.inc"
#include "th01/main/boss/sariel_top_rain.inc"
#include "th01/main/boss/sariel_radial_stacks.inc"
#include "th01/main/boss/sariel_dotted_circle.inc"
struct screen_point_t {screen_x_t x;screen_y_t y;};
#include "th01/main/boss/sariel_curved_spray.inc"
#include "th01/main/boss/sariel_double_spray.inc"
}
namespace th01_native {
SarielPattern::SarielPattern(std::uint32_t seed,SarielPatternKind kind,SarielBody* body):kind_(kind),body_(body){if(kind==SarielPatternKind::SnowflakeWithRings && !body)throw std::invalid_argument("Snowflake pattern requires Sariel body");if(active)throw std::logic_error("Sariel patterns support one instance");if(body)body->raise_wand(true);active=true;native_random_seed=seed;}
SarielPattern::~SarielPattern(){active=false;}
bool SarielPattern::advance(){if(done_)return true;boss_phase_frame=++frame_;active_body=body_;switch(kind_){case SarielPatternKind::CurvedSprayOnce:pattern_curved_spray_leftright_once(boss_phase_frame);break;case SarielPatternKind::CurvedSprayTwice:pattern_curved_spray_leftright_twice(boss_phase_frame);break;case SarielPatternKind::FourLasersWithRain:pattern_four_aimed_lasers();pattern_rain_from_top();break;case SarielPatternKind::RadialStacksAndLasers:pattern_radial_stacks_and_lasers();break;case SarielPatternKind::FourAimedLasers:pattern_four_aimed_lasers();break;case SarielPatternKind::RandomLasers:pattern_random_purple_lasers();break;case SarielPatternKind::OrbRings:pattern_2_rings_from_a2_orbs();break;case SarielPatternKind::SlingClusters:pattern_aimed_sling_clusters();break;case SarielPatternKind::Semicircles:pattern_four_semicircle_spreads();break;case SarielPatternKind::SnowflakeWithRings:pattern_detonating_snowflake();pattern_2_rings_from_a2_orbs();break;}frame_=boss_phase_frame;done_=frame_==0;return done_;}
}
