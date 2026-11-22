#include "particle_backend.hpp"
// Explicit DOS SoA aliases for index 40: x->y[0], y->vx[0], vx->vy[0],
// vy->alive[0..1], alive->velocity_base[0], velocity_base->spawn_cycle.
// Store bool truth values canonically; no bool object contains an invalid byte.
void native_particle_overflow(CParticles& p,particle_origin_t origin) {
    if(p.velocity_base[0]!=0) return;
    p.velocity_base[0]=1;
    p.spawn_cycle=(irand()%p.velocity_base_max)+1;
    int speed=p.spawn_cycle;
    switch(origin) {
    case PO_TOP:case PO_TOP_RIGHT:case PO_TOP_LEFT:
        p.y[0].v=TO_SP(irand()%640);p.velocity_x[0].v=0;break;
    case PO_BOTTOM_RIGHT:case PO_BOTTOM:case PO_BOTTOM_LEFT:
        p.y[0].v=TO_SP(irand()%640);p.velocity_x[0].v=TO_SP(399);break;
    case PO_RIGHT:case PO_LEFT:
        p.y[0].v=origin==PO_RIGHT ? TO_SP(639) : 0;p.velocity_x[0].v=TO_SP(irand()%400);break;
    default:throw std::invalid_argument("Particle origin");
    }
    int vx=0,vy=0;
    switch(origin) {
    case PO_TOP:vy=speed*16;break;
    case PO_TOP_RIGHT:vx=-speed*13;vy=speed*8;break;
    case PO_RIGHT:vx=-speed*16;break;
    case PO_BOTTOM_RIGHT:vx=-speed*13;vy=-speed*8;break;
    case PO_BOTTOM:vy=-speed*16;break;
    case PO_BOTTOM_LEFT:vx=speed*13;vy=-speed*8;break;
    case PO_LEFT:vx=speed*16;break;
    case PO_TOP_LEFT:vx=speed*13;vy=speed*8;break;
    }
    p.velocity_y[0].v=vx;
    auto word=std::uint16_t(vy);p.alive[0]=(word&255)!=0;p.alive[1]=(word>>8)!=0;
}