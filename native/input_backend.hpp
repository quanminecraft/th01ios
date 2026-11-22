#pragma once
#include "physics_types.hpp"
#include "th01/hardware/input.hpp"
#include "th01/main/player/bomb.hpp"
#include "platform/x86real/pc98/keyboard.hpp"

namespace th01_native {
// Physical state sampled once per gameplay tick, supplied by SDL or tests.
// The original input_sense implementation owns edge/consumption semantics.
struct InputKeys {
	bool up=false,down=false,left=false,right=false;
	bool num_up=false,num_down=false,num_left=false,num_right=false;
	bool shot=false,strike=false,escape=false,enter=false;
	bool roll_up=false,roll_down=false;
};
inline bool any_menu_input(const InputKeys& k) {
 return k.up||k.down||k.left||k.right||k.num_up||k.num_down||k.num_left||k.num_right||k.shot||k.strike||k.escape||k.enter;
}
void set_input_keys(const InputKeys& keys);
}
extern std::uint8_t end_flag;
constexpr std::uint8_t ES_NONE=0;
constexpr std::uint8_t ES_MAKAI=1,ES_JIGOKU=2;
extern bool16 mode_test;
int key_sense(int group);
void debug_mem();
void debug_show_game();
