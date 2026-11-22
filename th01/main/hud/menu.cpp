#include "platform/x86real/pc98/page.hpp"
#include "th01/hardware/grp2xscs.hpp"
#include "th01/math/clamp.hpp"
#include "th01/main/hud/menu.hpp"
#include "th01/shiftjis/hud.hpp"

#define pause_comp(color,i) (color).v[i]
#include "th01/main/hud/pause_menu.inc"
#undef pause_comp

#include "th01/main/bullet/pellet_s.cpp"

// Continue menu
// -------------

#include "th01/main/hud/continue_menu.inc"
