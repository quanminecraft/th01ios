#pragma once
#include "shot_backend.hpp"
namespace th01_native {
struct KikuriRippleBackend {Framebuffer& framebuffer;const SpriteSet& sprites;bool sloppy=false;};
class KikuriRippleBinding {
    KikuriRippleBackend* previous;
public:
    explicit KikuriRippleBinding(KikuriRippleBackend& value);
    ~KikuriRippleBinding();
    KikuriRippleBinding(const KikuriRippleBinding&)=delete;
    KikuriRippleBinding& operator=(const KikuriRippleBinding&)=delete;
};
void kikuri_ripple_advance(screen_x_t left,screen_y_t top,std::int8_t& frame);
bool kikuri_ripple_hit_test(screen_x_t left,pixel_t extra_width);
}
bool& native_kikuri_ripple_sloppy();
void native_kikuri_ripple_put(screen_x_t left,screen_y_t top,int id,int quarter);