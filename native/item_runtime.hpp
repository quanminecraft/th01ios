#pragma once
#include "item_motion.hpp"
#include "th01/math/clamp.hpp"
#include "th01/math/str_val.hpp"
#include "th01/main/stage/item.hpp"
#include "th01/common.h"
#include "shiftjis_width.hpp"
#define BOMB_COLLECT_1 "Bomb"
#define BOMB_COLLECT_2 "Extend!!"
// Original Shift-JIS bytes, independent of the host source-file encoding.
#define BOMB_COLLECT_CAP "\x82\x50\xe4\xdd"
extern std::uint16_t native_point_value;
