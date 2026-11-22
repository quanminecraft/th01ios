#pragma once
#include "missile_backend.hpp"
#include "boss_hit_backend.hpp"
#include "boss_entity_backend.hpp"
#include "hp_backend.hpp"
#include "line_backend.hpp"
#include "th01/main/bullet/line.hpp"
#include "th01/math/dir.hpp"
#include "th01/main/boss/yuugenmagan_types.hpp"
namespace th01_native {
struct YuugenmaganBackend {
    Framebuffer& framebuffer;
    MissileBackend& missiles;
    std::function<void()> music_fade,defeat;
    Palette4 palette{};
    bool defeated=false,reset_pending=true;
    int subphase=0,iterations=0,invincibility=0;
    unsigned eyes_open=0;
};
class YuugenmaganBinding {
    YuugenmaganBackend* previous;
public:
    explicit YuugenmaganBinding(YuugenmaganBackend& value);
    ~YuugenmaganBinding();
    YuugenmaganBinding(const YuugenmaganBinding&)=delete;
    YuugenmaganBinding& operator=(const YuugenmaganBinding&)=delete;
};
}
namespace yuugenmagan_original {
using dots8_t=std::uint8_t;
enum {COMPONENT_R=0,COMPONENT_G=1,COMPONENT_B=2};
using screen_point_t=th01_native::MimaPoint;
struct point_t{pixel_t x,y;};
extern yuugenmagan_int_t boss_hp,boss_phase_frame;
extern std::int8_t boss_phase;
void yuugenmagan_load();void yuugenmagan_setup();void yuugenmagan_main();void yuugenmagan_free();
void native_yuugenmagan_reset();bool native_yuugenmagan_before_frame();bool native_yuugenmagan_take_reset();
void native_yuugenmagan_observe(int subphase,int iterations,int invincibility,unsigned eyes);
void ptn_free(main_ptn_slot_t slot);void mdrv2_bgm_fade_out_nonblock();void boss_defeat_animate();
void boss_palette_snap();void boss_palette_show();
inline void graph_r_vline(screen_x_t x,screen_y_t top,screen_y_t bottom,vc2 color){graph_r_line(x,top,x,bottom,color);}
struct ComponentRef {
    std::int16_t& value;
    operator int()const{return value;}
    void change(int delta){unsigned byte=unsigned(int(value)+delta)&255u;value=byte<128 ? int(byte) : int(byte)-256;}
    ComponentRef& operator++(){change(1);return *this;}
    ComponentRef& operator--(){change(-1);return *this;}
    void operator++(int){change(1);}
    void operator--(int){change(-1);}
};
}
#define z_Palettes native_bomb_palette()
#define stage_palette const_cast<Palette4&>(native_bomb_stage_palette())
#define graph_r_line_unput native_graph_line_unput
#define inhibit_Z3(x) (x)
#define yuugenmagan_component(color,index) ComponentRef{(color).component(index)}