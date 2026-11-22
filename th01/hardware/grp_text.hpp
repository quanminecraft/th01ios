/// Font ROM glyphs in VRAM
/// -----------------------
/// TH01-exclusive extensions to the graph_putsa_fx() function.

#ifndef TH01_HARDWARE_GRP_TEXT_HPP
#define TH01_HARDWARE_GRP_TEXT_HPP

#ifdef TH01_NATIVE_ORB_PHYSICS
#include "native/shape_types.hpp"
using shiftjis_t=char;
#else
#include "pc98.h"
#include "shiftjis.hpp"
#endif

// Variadic version of graph_putsa_fx().
void graph_printf_fx(
	screen_x_t left,
	vram_y_t top,
	int16_t col_and_fx,
	const shiftjis_t *fmt,
	...
);

// Puts the rightmost N [digits] of [num] onto the graphics RAM, using
// full-width digits, and applying the given effect. (Consequently, the units
// place will appear at x = [left] + (([digits] - 1) * GLYPH_FULL_W).)
// Each digit is blitted separately; if [num_prev] is nonzero, the function
// only blits the digits of [num] that differ from those in [num_prev].
// Will put nothing if [put_leading_zeroes] is false and [num] is 0.
void graph_putfwnum_fx(
	screen_x_t left, vram_y_t top, int16_t col_and_fx, int digits,
#ifdef TH01_NATIVE_ORB_PHYSICS
	std::int32_t num, std::int32_t num_prev, bool16 put_leading_zeroes
#else
	long num, long num_prev, bool16 put_leading_zeroes
#endif
);
/// -----------------------

#endif /* TH01_HARDWARE_GRP_TEXT_HPP */
