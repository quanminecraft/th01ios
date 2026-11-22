#include "mima_entrance.hpp"
namespace {
constexpr int MIMA_W=128,MIMA_H=160;
th01_native::MimaEntrance* active=nullptr;
th01_native::MimaEntrance& current() {if(!active) throw std::logic_error("Mima entrance is not bound");return *active;}
#define ent_still current().entity
#define boss_phase_frame current().frame
#define spreadin_interval current().interval
#define spreadin_speed current().speed
void mima_bg_snap() {
    auto& state=current();auto& fb=th01_native::graphics_framebuffer();
    int left=ent_still.cur_left/8*8,top=ent_still.cur_top;
    for(int y=0;y<MIMA_H;++y) for(int x=0;x<MIMA_W;++x) state.background[y*MIMA_W+x]=fb.pixel(1,left+x,top+y);
    ++state.snapshots;
}
void mima_put_still_both() {page_access(1);ent_still.put_8();page_access(0);ent_still.put_8();}
#include "th01/main/boss/mima_spreadin.inc"
#undef ent_still
#undef boss_phase_frame
#undef spreadin_interval
#undef spreadin_speed
}
namespace th01_native {
bool MimaEntrance::advance(screen_x_t left,screen_y_t top) {
    if(!interval || !speed || speed/interval==0) throw std::invalid_argument("Non-progressing Mima entrance timing");
    if(left<0 || left+MIMA_W>640 || top<0 || top+MIMA_H>400) throw std::invalid_argument("Mima entrance outside framebuffer");
    auto* previous=active;active=this;
    try {++frame;phase_spreadin(left,top);active=previous;} catch(...) {active=previous;throw;}
    return frame==0;
}
}