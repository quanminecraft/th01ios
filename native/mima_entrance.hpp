#pragma once
#include "boss_entity_backend.hpp"
namespace th01_native {
struct MimaEntrance {
    CBossEntity entity{};
    std::int16_t frame=0;
    std::uint8_t interval=1,speed=2;
    std::array<std::uint8_t,128*160> background{};
    unsigned snapshots=0;
    // Call once per original entrance loop iteration. Completion is frame=0;
    // the caller waits two frames after every non-completing iteration.
    bool advance(screen_x_t left,screen_y_t top);
};
}