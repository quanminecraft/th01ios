#pragma once
#include "assets.hpp"
#include "player_backend.hpp"
#include "th01/main/player/player.hpp"
#include "math_backend.hpp"
#include "th01/math/overlap.hpp"
#include "th01/main/bullet/missile.hpp"
namespace th01_native {
struct MissileBackend {
    const std::vector<Entry>& archive;
    SpriteSet sprites;
    int slot=-1;
    std::int16_t frame=0;
};
class MissileBinding {
    MissileBackend* previous;
public:
    explicit MissileBinding(MissileBackend& value);
    ~MissileBinding();
    MissileBinding(const MissileBinding&)=delete;
    MissileBinding& operator=(const MissileBinding&)=delete;
};
}
std::int16_t native_missile_frame();
void native_missile_put(screen_x_t left,screen_y_t top,int id,int quarter);