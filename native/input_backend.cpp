#include "input_backend.hpp"
#include <array>
#include <stdexcept>

std::uint8_t end_flag=ES_NONE;
bool16 mode_test=false;
int16_t bomb_doubletap_frame=0;
namespace {std::array<std::uint8_t,15> groups{};}
namespace th01_native {
void set_input_keys(const InputKeys& keys) {
	groups.fill(0);
	if(keys.up) groups[7]|=K7_ARROW_UP;
	if(keys.down) groups[7]|=K7_ARROW_DOWN;
	if(keys.left) groups[7]|=K7_ARROW_LEFT;
	if(keys.right) groups[7]|=K7_ARROW_RIGHT;
	if(keys.num_up) groups[8]|=K8_NUM_8;
	if(keys.num_down) groups[9]|=K9_NUM_2;
	if(keys.num_left) groups[8]|=K8_NUM_4;
	if(keys.num_right) groups[9]|=K9_NUM_6;
	if(keys.shot) groups[5]|=K5_Z;
	if(keys.strike) groups[5]|=K5_X;
	if(keys.escape) groups[0]|=K0_ESC;
	if(keys.enter) groups[3]|=K3_RETURN;
	if(keys.roll_up) groups[6]|=K6_ROLL_UP;
	if(keys.roll_down) groups[6]|=K6_ROLL_DOWN;
}
}
int key_sense(int group) {return groups.at(group);}
void debug_mem() {throw std::runtime_error("Original DOS heap debug screen is unavailable in the native build");}
void debug_show_game() {throw std::runtime_error("Original DOS heap debug screen is unavailable in the native build");}
