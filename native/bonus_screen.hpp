#pragma once
#include "font_rom.hpp"
#include "bonus_values.hpp"
#include <functional>
namespace th01_native {
// Original regular-stage box geometry, stipple, labels and fullwidth digits.
// The caller commits score/extend and handles original input acknowledgement.
void render_stage_bonus(Framebuffer& fb,const FontRom& font,unsigned stage,const StageBonusValues& values,const std::function<void(unsigned)>& wait);
}
