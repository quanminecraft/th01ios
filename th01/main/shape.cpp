#include "platform/x86real/pc98/page.hpp"
#include "th01/v_colors.hpp"
#include "th01/math/polar.hpp"
#include "th01/hardware/egc.h"
#include "th01/sprites/shape8x8.hpp"
#include "th01/main/shape.hpp"

#include "th01/sprites/shape8x8.csp"

#define shape8x8_put(shape, left, top, col) \
	dot_rect_t(8, 8) sprite = sSHAPE8X8[shape]; \
	vram_offset_t vram_offset_topleft = vram_offset_divmul(left, top); \
	int first_bit = (left % BYTE_DOTS); \
	if((left < 0) || (left >= RES_X) || (top < 0) || (top >= RES_Y)) { \
		return; \
	} \
	grcg_put_8x8_mono(vram_offset_topleft, first_bit, sprite.row, col);

void shape8x8_diamond_put(screen_x_t left, vram_y_t top, vc2 col)
{
	shape8x8_put(SHAPE8X8_DIAMOND, left, top, col);
}

void shape8x8_star_put(screen_x_t left, vram_y_t top, vc2 col)
{
	shape8x8_put(SHAPE8X8_STAR, left, top, col);
}

void shape8x8_flake_put(screen_x_t left, vram_y_t top, vc2 col)
{
	shape8x8_put(SHAPE8X8_FLAKE, left, top, col);
}

#include "th01/main/shape_arc.inc"

#include "th01/main/shape_arc_unput.inc"

void shape8x8_invincibility_put_with_mask_from_B_plane(
	screen_x_t left, vram_y_t top, int cel
);

void shape8x8_invincibility_put(screen_x_t left, vram_y_t top, int cel)
{
	if(left < 0) {
		return;
	}
	shape8x8_invincibility_put_with_mask_from_B_plane(left, top, cel);

	vram_offset_t vram_offset = vram_offset_divmul(left, top);
	int first_bit = (left % BYTE_DOTS);

	if(cel > (SHAPE8X8_INVINCIBILITY_CELS - 1)) {
		return;
	}

	grcg_setcolor_rmw(V_RED);
	for(pixel_t y = 0; y < sSHAPE8X8[0].h(); y++) {
		#define sprite sSHAPE8X8[SHAPE8X8_INVINCIBILITY + cel][y]

		if(first_bit == 0) {
			grcg_put(vram_offset, sprite, 8);
		} else {
			grcg_put(
				vram_offset, (
					(sprite << ((BYTE_DOTS * 2) - first_bit)) +
					(sprite >> first_bit)
				),
				16
			);
		}
		vram_offset += ROW_SIZE;
		if(vram_offset > PLANE_SIZE) { // Clip at the bottom edge
			break;
		}

		#undef sprite
	}
	grcg_off_func();
}

// Surely this function was meant to just regularly unblit the sprite via the
// EGC, or even the GRCG's TCR mode? The RMW mode has no effect on VRAM reads,
// and simply returns the exact bytes at the given offset on the given (that
// is, the B) plane. As a result, this unblitting attempt actually blits the
// sprite again, masked by whatever was in VRAM plane B at the given position
// before calling this function.
void shape8x8_invincibility_put_with_mask_from_B_plane(
	screen_x_t left, vram_y_t top, int cel
)
{
	if(left < 0) {
		return;
	}

	vram_offset_t vram_offset = vram_offset_divmul(left, top);
	int first_bit = (left % BYTE_DOTS);

	if(cel > (SHAPE8X8_INVINCIBILITY_CELS - 1)) {
		return;
	}

	grcg_setcolor_rmw(V_RED);
	for(pixel_t y = 0; y < sSHAPE8X8[0].h(); y++) {
		#define sprite sSHAPE8X8[SHAPE8X8_INVINCIBILITY + cel][y]

		if(first_bit == 0) {
			dots8_t bg_B;

			page_access(1);	bg_B = (grcg_chunk_8(vram_offset) & sprite);
			page_access(0);	grcg_put_8(vram_offset, bg_B);
		} else {
			// MODDERS: Add clipping at the right edge
			dots8_t bg_B_left;
			dots8_t bg_B_right;

			page_access(1);
			bg_B_left = (grcg_chunk_8(vram_offset + 0) & (sprite >> first_bit));

			page_access(1);
			bg_B_right = (
				grcg_chunk_8(vram_offset + 1) &
				(sprite << (BYTE_DOTS - first_bit))
			);

			page_access(0); grcg_put_8((vram_offset + 0), bg_B_left);
			page_access(0); grcg_put_8((vram_offset + 1), bg_B_right);
		}

		vram_offset += ROW_SIZE;
		if(vram_offset > PLANE_SIZE) { // Clip at the bottom edge
			break;
		}
		#undef sprite
	}
	grcg_off_func();
}
