#pragma once
#include "bomb_backend.hpp"
#include "card_backend.hpp"
#include "font_rom.hpp"
#include "input_backend.hpp"
namespace th01_native {
struct ContinueResident {
	std::int32_t score_highest=0,score=0;
	std::int8_t credit_lives_extra=2,rem_lives=0;
	bool snd_need_init=true;
	std::uint16_t point_value=0;
};
struct ContinueBackend {
	Framebuffer& framebuffer;
	const FontRom& font;
	ContinueResident& resident;
	std::uint16_t& continues;
	std::function<InputKeys()> poll;
	std::function<void(unsigned)> wait;
	std::function<void()> stop_music;
};
class ContinueBinding {
	ContinueBackend* previous;
public:
	explicit ContinueBinding(ContinueBackend& backend);
	~ContinueBinding();
	ContinueBinding(const ContinueBinding&)=delete;
	ContinueBinding& operator=(const ContinueBinding&)=delete;
};
}
// Native true requests the original MAIN reload; false returns to title.
bool16 continue_menu();
