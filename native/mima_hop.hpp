#pragma once
#include "physics_types.hpp"
#include <memory>
namespace th01_native {
struct MimaHopState {screen_x_t left;screen_y_t top;std::int16_t frame;bool hitbox_inactive,meteor;unsigned snapshots;};
class MimaHopPattern {
    struct Impl;
    std::unique_ptr<Impl> impl;
public:
    MimaHopPattern();
    ~MimaHopPattern();
    MimaHopPattern(const MimaHopPattern&)=delete;
    MimaHopPattern& operator=(const MimaHopPattern&)=delete;
    bool advance();
    void restart();
    MimaHopState state() const;
};
}