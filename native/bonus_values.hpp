#pragma once
#include "physics_types.hpp"
#include "th01/main/bonus_values.hpp"
#include <array>
namespace th01_native {
struct StageBonusValues {
	std::array<std::int16_t,4> metrics;
	std::uint32_t total;
};
// Assignment to original DOS int precedes the signed upper clamp. In
// particular, overflow does not saturate and negative values are not clamped.
inline std::int16_t bonus_dos_int(int value) {
	unsigned bits=static_cast<unsigned>(value)&0xffffu;
	return static_cast<std::int16_t>(bits<0x8000 ? int(bits) : int(bits)-0x10000);
}
inline StageBonusValues stage_bonus_values(std::uint16_t timer,std::int16_t combo,std::int16_t lives,std::int8_t bombs,std::int16_t stage) {
	StageBonusValues out{{bonus_dos_int(stagebonus_time_value(timer)),bonus_dos_int(stagebonus_combo_value(combo)),bonus_dos_int(stagebonus_resources_value(lives,bombs)),bonus_dos_int(stagebonus_stage_value(stage))},0};
	for(auto& value:out.metrics) {
		if(value>6553) value=6553;
		for(unsigned i=0;i<10;++i) out.total+=static_cast<std::uint32_t>(value);
	}
	return out;
}
}
