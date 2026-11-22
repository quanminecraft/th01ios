#include "kikuri_ripple.hpp"
#include "th01/main/player/player.hpp"
namespace {
using vram_y_t=std::int16_t;
th01_native::KikuriRippleBackend* active=nullptr;
th01_native::KikuriRippleBackend& backend() {if(!active) throw std::runtime_error("Kikuri ripple backend is not bound");return *active;}
constexpr int RIPPLE_W=16,PTN_RIPPLE=PTN_ID(PTN_SLOT_BOSS_1,0);
enum {C_RIPPLE_LOW=0,C_RIPPLE_HIGH=1,C_RIPPLE_FILLED=2,C_RIPPLE_EMPTY=3};
#define ptn_sloppy_unput_before_alpha_put native_kikuri_ripple_sloppy()
#define ptn_put_quarter_8 native_kikuri_ripple_put
#include "th01/main/boss/kikuri_ripple_hit.inc"
#include "th01/main/boss/kikuri_ripple.inc"
#undef ptn_put_quarter_8
#undef ptn_sloppy_unput_before_alpha_put
}
namespace th01_native {
KikuriRippleBinding::KikuriRippleBinding(KikuriRippleBackend& value):previous(active) {
    if(value.sprites.images.empty() || value.sprites.images[0].width!=32 || value.sprites.images[0].height!=32) throw std::invalid_argument("Kikuri ripple requires a quartered PTN sprite");
    if(&value.framebuffer!=&graphics_framebuffer()) throw std::invalid_argument("Kikuri ripple framebuffer mismatch");active=&value;
}
KikuriRippleBinding::~KikuriRippleBinding() {active=previous;}
void kikuri_ripple_advance(screen_x_t left,screen_y_t top,std::int8_t& frame) {ripple_update_and_render(left,top,frame);}
bool kikuri_ripple_hit_test(screen_x_t left,pixel_t extra_width) {return tear_ripple_hittest(left,extra_width);}
}
bool& native_kikuri_ripple_sloppy() {return backend().sloppy;}
void native_kikuri_ripple_put(screen_x_t left,screen_y_t top,int id,int quarter) {
    if(id!=PTN_RIPPLE) throw std::invalid_argument("Unexpected Kikuri ripple PTN id");
    auto& b=backend();
    if(b.sloppy) ptn_sloppy_unput_quarter_16(left,top);
    th01_native::ShotBackend ripple{b.framebuffer,b.sprites,{},th01_native::current_render_page()};
    th01_native::ShotBackendBinding binding(ripple);ptn_put_quarter_8(left,top,0,quarter);
}