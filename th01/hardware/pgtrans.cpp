#pragma option -zCGRAPH_EX_TEXT

#include <dos.h>
#include "platform/x86real/pc98/page.hpp"
#include "th01/sprites/ileave_m.hpp"
#include "th01/hardware/palette.h"
#include "th01/hardware/pgtrans.hpp"

// Combines the pixels from both VRAM pages in the 8×8 square starting at
// (⌊left/8⌋*8, top), using one of the interlace masks. The resulting square
// is blitted to VRAM page 0.
void graph_interleave_pages_8x8_8(screen_x_t left, vram_y_t top, int mask_id)
{
	#include "th01/sprites/ileave_m.csp"

	vram_offset_t vo = vram_offset_divmul(left, top);
	dots_t(INTERLEAVE_W) dots;
	for(char y = 0; y < INTERLEAVE_H; y++) {
		#define mask sINTERLEAVE_MASKS[mask_id][y]

		#define snap(plane, vo) \
			VRAM_CHUNK(plane, vo, 8)

		#define interleave(plane, vram_offset) \
			page_access(1); dots  = ((~mask) & snap(plane, vo)); \
			page_access(0); dots |= (  mask  & snap(plane, vo)); \
			page_access(0); VRAM_PUT(plane, vo, dots, 8);

		interleave(B, vo);
		interleave(R, vo);
		interleave(G, vo);
		interleave(E, vo);
		vo += ROW_SIZE;

		#undef interleave
		#undef snap
		#undef mask
	}
}

void pagetrans_diagonal_8x8(unsigned int step_ms)
{
	pagetrans_diagonal_8x8_with_palette(step_ms, z_Palettes, z_Palettes);
}

#include "th01/hardware/pgtrans_diagonal.inc"
