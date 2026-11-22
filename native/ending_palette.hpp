#pragma once
#include "framebuffer.hpp"
#include <functional>
#pragma push_macro("z_Palettes")
#undef z_Palettes

namespace th01_native {
// Original GRP palette tone and fade timing, owned by one ending scene.
class EndingPalette {
 using vc_t=unsigned;
 struct RGB4 { int v[3]{}; static constexpr int max() {return 15;} };
 using Palette4=std::array<RGB4,16>;
 static constexpr int COLOR_COUNT=16,COMPONENT_COUNT=3,TONE_STEP_PER_FRAME=5;
 Palette4 grp_palette{},z_Palettes{};
 int grp_palette_tone=100;
 Framebuffer& framebuffer;
 std::function<void(unsigned)> wait;
 void z_palette_set_all_show(const Palette4& colors);
 void frame_delay(unsigned frames) {wait(frames);}
public:
 EndingPalette(Framebuffer& framebuffer,std::function<void(unsigned)> wait);
 // Loading a new GRP preserves the current tone, exactly as grp_put does.
 void load(const Palette& colors);
 int tone() const {return grp_palette_tone;}
#include "th01/formats/grp_palette.inc"
};
}
#pragma pop_macro("z_Palettes")
