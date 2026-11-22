#include "yuugenmagan_backend.hpp"
namespace {
th01_native::YuugenmaganBackend* active=nullptr;
th01_native::YuugenmaganBackend& backend(){if(!active)throw std::logic_error("YuugenMagan backend is not bound");return *active;}
}
namespace th01_native {
YuugenmaganBinding::YuugenmaganBinding(YuugenmaganBackend& value):previous(active){
    if(active)throw std::logic_error("YuugenMagan supports one active instance");
    if(!value.music_fade || !value.defeat)throw std::invalid_argument("YuugenMagan callbacks are required");
    if(&value.framebuffer!=&graphics_framebuffer())throw std::invalid_argument("YuugenMagan framebuffer mismatch");active=&value;
}
YuugenmaganBinding::~YuugenmaganBinding(){active=previous;}
}
namespace yuugenmagan_original {
yuugenmagan_int_t boss_hp=0,boss_phase_frame=0;std::int8_t boss_phase=0;
void native_yuugenmagan_reset(){auto& b=backend();b.defeated=false;b.reset_pending=true;b.subphase=b.iterations=b.invincibility=0;b.eyes_open=0;}
bool native_yuugenmagan_take_reset(){auto& b=backend();bool reset=b.reset_pending;b.reset_pending=false;return reset;}
bool native_yuugenmagan_before_frame(){auto& b=backend();b.missiles.frame=std::int16_t(native_boss_frame());return !b.defeated;}
void native_yuugenmagan_observe(int subphase,int iterations,int invincibility,unsigned eyes){auto& b=backend();b.subphase=subphase;b.iterations=iterations;b.invincibility=invincibility;b.eyes_open=eyes;}
void ptn_free(main_ptn_slot_t slot){auto& m=backend().missiles;if(m.slot!=slot)throw std::invalid_argument("YuugenMagan missile slot mismatch");m.sprites={};m.slot=-1;}
void mdrv2_bgm_fade_out_nonblock(){backend().music_fade();}
void boss_defeat_animate(){auto& b=backend();if(!b.defeated){b.defeated=true;b.defeat();}}
void boss_palette_snap(){backend().palette=native_bomb_palette();}
void boss_palette_show(){z_palette_set_all_show(backend().palette);}
}