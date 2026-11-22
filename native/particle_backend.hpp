#pragma once
#include "shot_backend.hpp"
#include "th01/main/particle.hpp"
#include "th01/math/overlap.hpp"
using uscreen_x_t=std::uint16_t;using uscreen_y_t=std::uint16_t;
constexpr int RES_X=640,RES_Y=400;
class GrpMono {
    std::uint8_t color=0;
public:
    void set_color(vc_t value) {if(value>15) throw std::invalid_argument("Particle color");color=value;}
    void draw_point_noclip(screen_x_t x,screen_y_t y) {th01_native::graphics_framebuffer().put(th01_native::current_render_page(),x,y,color);}
};
void native_particle_overflow(CParticles& particles,particle_origin_t origin);