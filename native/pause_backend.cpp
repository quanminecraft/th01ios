#include "pause_backend.hpp"
#include "pause_strings.inc"
namespace {
th01_native::PauseBackend* active=nullptr;
th01_native::PauseBackend& backend() {if(!active) throw std::runtime_error("Native pause backend is not bound");return *active;}
void native_pause_sense(bool16 check) {th01_native::set_input_keys(backend().poll());input_sense(check);}
void frame_delay(unsigned n) {backend().delay(n);}
void resident_continue_use() {++backend().continues_for_scene;}
}
namespace th01_native {
PauseBackendBinding::PauseBackendBinding(PauseBackend& value):previous(active) {
	if(!value.poll || !value.delay) throw std::invalid_argument("Pause input/timing integrations are incomplete");
	active=&value;
}
PauseBackendBinding::~PauseBackendBinding() {active=previous;}
}
#define input_sense native_pause_sense
#define z_Palettes native_bomb_palette()
#define stage_palette native_bomb_stage_palette()
#define pause_comp(color,i) (color).component(i)
#include "th01/main/hud/pause_menu.inc"
