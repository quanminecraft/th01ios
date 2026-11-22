#pragma once
#include "bomb_backend.hpp"
#include "fullwidth_text.hpp"
#include "input_backend.hpp"
#include "shiftjis_width.hpp"
#include "th01/main/playfld.hpp"
#include "th01/math/clamp.hpp"
using svc2=std::int16_t;
namespace th01_native {
struct PauseBackend {
	std::function<InputKeys()> poll;
	std::function<void(unsigned)> delay;
	std::uint16_t& continues_for_scene;
};
class PauseBackendBinding {
	PauseBackend* previous;
public:
	explicit PauseBackendBinding(PauseBackend& backend);
	~PauseBackendBinding();
	PauseBackendBinding(const PauseBackendBinding&)=delete;
	PauseBackendBinding& operator=(const PauseBackendBinding&)=delete;
};
}
bool16 pause_menu();
