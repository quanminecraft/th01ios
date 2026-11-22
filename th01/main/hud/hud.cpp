#include "th01/rank.h"
#include "th01/resident.hpp"
#include "th01/v_colors.hpp"
#include "th01/math/str_val.hpp"
#include "th01/hardware/graph.h"
#include "th01/hardware/grppsafx.h"
#include "th01/hardware/grp_text.hpp"
#include "th01/hardware/grppsafx.h"
#include "th01/formats/grf.hpp"
#include "th01/main/player/player.hpp"
#include "th01/main/hud/hud.hpp"
#include "th01/main/stage/stages.hpp"
#include "th01/main/stage/timer.hpp"
#include "platform/x86real/pc98/page.hpp"

/// Constants
/// ---------
static const screen_x_t LIVES_LEFT = 128;
static const screen_y_t LIVES_TOP = 0;
static const int LIVES_PER_ROW = 6; // for all the cheaters?

static const screen_x_t BOMBS_LEFT = 128;
static const screen_y_t BOMBS_TOP = 16;

static const screen_x_t SCORE_LEFT = 256;
static const screen_y_t MAX_TOP = 0;
static const screen_y_t CUR_TOP = 16;

static const screen_x_t STAGE_LEFT = 608;
static const screen_y_t STAGE_TOP = 32;

static const screen_x_t RANK_CENTER_X = 600;
static const screen_y_t RANK_TOP = 48;

static const int PTN_LIFE_QUARTER = 0;
static const int PTN_BOMB_QUARTER = 1;

static const int16_t MAX_FX = (V_WHITE | FX_WEIGHT_BOLD);
static const int16_t CUR_FX = (V_WHITE | FX_WEIGHT_BLACK);

#if (PTN_QUARTER_W < GLYPH_FULL_W)
#error Original code assumes PTN_QUARTER_W >= GLYPH_FULL_W
#endif
#if (PTN_QUARTER_H < GLYPH_H)
#error Original code assumes PTN_QUARTER_H >= GLYPH_H
#endif
static const pixel_t COL_W = PTN_QUARTER_W;
static const pixel_t ROW_H = PTN_QUARTER_H;

static const pixel_t SCORE_W = (SCORE_DIGITS * COL_W);

static const screen_x_t CARDCOMBO_LEFT = (
	SCORE_LEFT + ((SCORE_DIGITS + 2) * COL_W)
);
static const pixel_t CARDCOMBO_W = (CARDCOMBO_DIGITS * COL_W);
static const screen_x_t CARDCOMBO_RIGHT = (CARDCOMBO_LEFT + CARDCOMBO_W);

static const pixel_t SCORE_AND_CARDCOMBO_W = (CARDCOMBO_RIGHT - SCORE_LEFT);
/// ---------

/// Globals
/// -------

// Forces re-rendering of all full-width numbers on the HUD, even if they
// haven't changed since the last render call.
bool fwnum_force_rerender = false;

uint8_t *hud_bg;
uint8_t hud_bg_rle_run_byte;
unsigned char hud_cardcombo_max; // Why a separate variable???
size_t hud_bg_size;

unsigned int hud_hp_first_white;
unsigned int hud_hp_first_redwhite;
/// -------

/// Functions
/// ---------

inline screen_x_t col_left(screen_x_t first_left, int col) {
	return (first_left + (col * COL_W));
}

#define bg_put(first_left, col, top, ptn_id, quarter) \
	ptn_put_quarter_noalpha_8(col_left(first_left, col), top, ptn_id, quarter)

#define bg_snap(first_left, col, top, ptn_id, quarter) \
	ptn_snap_quarter_8(col_left(first_left, col), top, ptn_id, quarter)

#define sprite_put(first_left, col, top, ptn_id, quarter) \
	ptn_put_quarter_8(col_left(first_left, col), top, ptn_id, quarter)

#define ptn_id_and_quarter_from_i(func, first_left, col, top, ptn_id_base, i) \
	func(first_left, col, top, (ptn_id_base + (i / 4)), (i % 4))

#define digit_changed(var, var_prev, divisor) \
	((var_prev / divisor) % 10) != ((var / divisor) % 10) || \
	(fwnum_force_rerender == true)

// Copies the (⌊[w]/16⌋*16)×[ROW_H] pixels starting at (⌊left/8⌋*8, top) from
// VRAM page 0 to VRAM page 1.
void graph_copy_hud_row_0_to_1_8(screen_x_t left, vram_y_t top, pixel_t w);
/// ---------

#define resident_hiscore resident->hiscore
#include "th01/main/hud/hud_score.inc"
#undef resident_hiscore

/// Background
/// ----------
void hud_bg_put(void)
{
	register uint16_t vram_offset;
	uint16_t src_offset = 0;
	uint8_t byte;
	uint8_t runs;
	uint8_t rle_run_byte = hud_bg_rle_run_byte;

	disable();
	grcg_setcolor_rmw(V_RED);
	for(vram_offset = 0; vram_offset < (66 * ROW_SIZE); vram_offset++) {
		// Effectively the same algorithm as used for .GRX files, except for
		// • the customizable RLE run byte, and
		// • implicitly defining a run of 0x00 dots as a skip. (Which really
		//   is just an optimization here, since blitting a 0x00 byte with the
		//   GRCG won't blit anything after all.)
		byte = hud_bg[src_offset++];
		if(byte == rle_run_byte) {
			runs = hud_bg[src_offset++];
			if(runs) {
				byte = hud_bg[src_offset++];
				while(runs--) {
					if(byte != 0) {
						grcg_put(vram_offset, byte, 8);
					}
					vram_offset++;
				}
			}
		}
		// Yes, in case we just did a putting run, we are in fact putting
		// one more byte!
		if(byte != 0) {
			grcg_put(vram_offset, byte, 8);
		}
	}
	grcg_off_func();
	enable();
}

int hud_bg_load(const char *fn)
{
	int size;
	grf_header_t header;

	file_ropen(fn);
	file_read(&header, sizeof(header));

	hud_bg_rle_run_byte = header.rle_run_byte;
	size = header.rle_size.B;
	hud_bg_size = size;
	hud_bg = new uint8_t[size];

	file_read(hud_bg, size);
	file_close();
	return 0;
}
/// ----------

void graph_copy_hud_row_0_to_1_8(screen_x_t left, vram_y_t top, pixel_t w)
{
	vram_offset_t vram_offset;
	dots16_t dots;
	pixel_t y;
	vram_offset_t vram_offset_row = vram_offset_divmul(left, top);
	vram_word_amount_t x;

	vram_offset = vram_offset_row;
	for(y = 0; y < ROW_H; y++) {
		vram_offset = vram_offset_row;
		for(x = 0; x < (w / 16); x++) {
			#define copy_plane(plane) \
				outport(0xA6, 0);	VRAM_SNAP(dots, plane, vram_offset, 16); \
				outport(0xA6, 1);	VRAM_PUT(plane, vram_offset, dots, 16);
			copy_plane(B);
			copy_plane(R);
			copy_plane(G);
			copy_plane(E);
			vram_offset += static_cast<vram_offset_t>(sizeof(dots));
			#undef copy_plane
		}
		vram_offset_row += ROW_SIZE;
	}
	page_access(0);
}

/// Lives and bombs
/// ---------------
#include "th01/main/hud/hud_icons.inc"

#include "th01/main/hud/hud_layout.inc"
