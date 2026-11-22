#include "boss_hit_backend.hpp"
namespace {
th01_native::BossHitBackend* active=nullptr;
th01_native::BossHitBackend& backend() {if(!active) throw std::runtime_error("Boss hit integrations are not bound");return *active;}
}
namespace th01_native {
BossHitBinding::BossHitBinding(BossHitBackend& value):previous(active) {
	if(!value.hp_decrement || !value.palette_show) throw std::invalid_argument("Boss HP/palette callbacks are required");active=&value;
}
BossHitBinding::~BossHitBinding() {active=previous;}
}
std::uint32_t& native_boss_frame() {return backend().frame;}
void native_boss_hp_decrement(int hp) {backend().hp_decrement(hp);}
void boss_palette_show() {backend().palette_show();}
#define rem_lives backend().lives
#include "th01/main/player/bomb_d_f.cpp"
