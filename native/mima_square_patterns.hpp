#pragma once
#include "physics_types.hpp"
#include <array>
#include <functional>
#include <memory>
namespace th01_native {
struct MimaPoint {screen_x_t x;screen_y_t y;};
enum class MimaSquareAttack {pellets,missiles,two_squares,halfcircle_missiles,slow_spray,lasers};
// Optional observer/renderer callback before and after rotation. An empty
// callback uses the native original line renderer. Projectiles are spawned.
using MimaSquareLines=std::function<void(bool,const std::array<MimaPoint,4>&)>;
class MimaSquarePattern {
    struct Impl;
    std::unique_ptr<Impl> impl;
public:
    MimaSquarePattern(MimaSquareAttack attack,screen_x_t left,screen_y_t top,MimaSquareLines lines={},std::function<void(bool)> casting={});
    ~MimaSquarePattern();
    MimaSquarePattern(const MimaSquarePattern&)=delete;
    MimaSquarePattern& operator=(const MimaSquarePattern&)=delete;
    bool advance();
    unsigned frame() const;
};
}