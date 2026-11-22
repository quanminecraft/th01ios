#include "math_backend.hpp"
#include <stdexcept>
#include "math_tables.inc"

std::int16_t iatan2(std::int16_t y,std::int16_t x) {
	if(!x && !y) return 0;
	std::uint16_t ay=y<0 ? -std::int32_t(y) : y;
	std::uint16_t ax=x<0 ? -std::int32_t(x) : x;
	unsigned angle;
	if(ax==ay) angle=32;
	else {
		// The original JL compares the wrapped 16-bit absolute values as
		// signed. Keep that unusual -32768 behavior and DIV failure boundary.
		bool vertical=static_cast<std::int16_t>(ax)<static_cast<std::int16_t>(ay);
		auto denominator=vertical ? ay : ax;
		if(!denominator) throw std::domain_error("Original iatan2 division by zero");
		std::uint32_t quotient=(std::uint32_t(vertical ? ax : ay)*256)/denominator;
		if(quotient>65535) throw std::overflow_error("Original iatan2 division overflow");
		angle=AtanTable8[quotient&255];
		if(vertical) angle=(64-angle)&255;
	}
	if(x<0) angle=128-angle;
	if(y<0) angle=0-angle;
	return angle&255;
}
