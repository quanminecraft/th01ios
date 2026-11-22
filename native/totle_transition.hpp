#pragma once
#include "bomb_backend.hpp"
namespace th01_native {
// Original alternating 32-dot row shifts and gradual palette transition.
void totle_transition(Framebuffer& fb,const SpriteSet& clear_screen,const std::function<void(unsigned)>& wait);
}
