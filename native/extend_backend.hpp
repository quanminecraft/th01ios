#pragma once
#include "card_backend.hpp"
#include "pellet_speed_binding.hpp"
#include "th01/main/extend.hpp"
namespace th01_native {
struct ExtendBackend {
	std::int16_t& lives;
	std::int8_t& saved_lives;
	std::function<void(int)> lives_hud;
 std::function<void()> threshold;
};
class ExtendBackendBinding {
	ExtendBackend* previous;
public:
	explicit ExtendBackendBinding(ExtendBackend& backend);
	~ExtendBackendBinding();
	ExtendBackendBinding(const ExtendBackendBinding&)=delete;
	ExtendBackendBinding& operator=(const ExtendBackendBinding&)=delete;
};
}
std::int16_t& native_extend_lives();
std::int8_t& native_extend_saved_lives();
void hud_lives_put(int previous);
void native_extend_threshold();
