#include <dos.h>
#include "libs/master.lib/pc98_gfx.hpp"
#include "th01/resident.hpp"
#include "th01/v_colors.hpp"
#include "th01/math/clamp.hpp"
#include "th01/hardware/egc_impl.hpp"
#include "th01/hardware/frmdelay.h"
#include "th01/hardware/graph.h"
#include "th01/hardware/grppsafx.h"
#include "th01/hardware/input.hpp"
#include "th01/hardware/palette.h"
#include "th01/snd/mdrv2.h"
#include "th01/formats/grp.h"
#include "th01/shiftjis/fwnum.hpp"
#include "th01/shiftjis/bonus.hpp"
#include "th01/sprites/bonusbox.hpp"
#include "th01/sprites/bonusbox.csp"
#include "th01/main/extend.hpp"
#include "th01/main/hud/hud.hpp"
#include "th01/main/bonus.hpp"
#include "th01/main/bonus_values.hpp"
#include "th01/main/bullet/pellet.hpp"
#include "th01/main/player/player.hpp"
#include "th01/main/player/shot.hpp"
#include "th01/main/stage/palette.hpp"
#include "th01/main/stage/stages.hpp"
#include "platform/x86real/pc98/page.hpp"

/// Coordinates
/// -----------

// Stage bonus
// -----------

static const screen_x_t STAGEBONUS_PADDED_LEFT = 32;
static const screen_y_t STAGEBONUS_PADDED_TOP = 40;
static const int STAGEBONUS_ROWS = 14;
static const pixel_t STAGEBONUS_PADDING_TOP = 40;
static const pixel_t STAGEBONUS_PADDED_W = ((2 * GLYPH_FULL_W) + STAGEBONUS_W);
static const pixel_t STAGEBONUS_PADDED_H = (
	STAGEBONUS_PADDING_TOP + (STAGEBONUS_ROWS * GLYPH_H) + GLYPH_H
);
// -----------

// TOTLE
// -----

#include "th01/main/totle_coords.inc"
// -----
/// -----------

// State
// -----

uscore_t score_bonus = 0; // Should be local to the respective function.

#include "th01/main/bonus_accumulate.inc"
// -----

#include "th01/main/totle_digits.inc"
static const int ROWSHIFT_CHUNK_DOTS = (EGC_REGISTER_DOTS * 2);
static const int ROWSHIFT_CHUNK_SIZE = (EGC_REGISTER_SIZE * 2);

// Shifts [ROWSHIFT_CHUNK_DOTS] pixels of the given row on VRAM page 0 from
// either the right edge of VRAM to the left or vice versa depending on whether
// [y] is even or odd, then fills the freed-up chunk with a chunk of pixels
// from VRAM page 1 that start [transferred_offset] bytes away from the shifted
// edge.
// Assumes that VRAM page 0 is accessed, and that the EGC is active and
// initialized for a copy.
void near egc_pagetrans_rowshift_alternating(
	vram_y_t y, vram_byte_amount_t transferred_offset
)
{
	enum {
		CHUNK_OFFSET_LEFT  = (0 * EGC_REGISTER_SIZE),
		CHUNK_OFFSET_RIGHT = (1 * EGC_REGISTER_SIZE),
	};

	uvram_offset_t vo_p0;
	egc_temp_t dots;
	vram_byte_amount_t negative_chunk_delta; // negative for extra confusion?
	union {
		vram_dword_amount_t num;
		vram_offset_t vo_p1;
	} u1;

	// Shift
	if(y & 1) {
		vo_p0 = vram_offset_muldiv((RES_X - ROWSHIFT_CHUNK_DOTS), y);
		negative_chunk_delta = -ROWSHIFT_CHUNK_SIZE;
	} else {
		vo_p0 = vram_offset_muldiv(0, y);
		negative_chunk_delta = ROWSHIFT_CHUNK_SIZE;
	}
	for(u1.num = 0; u1.num < ((ROW_SIZE / ROWSHIFT_CHUNK_SIZE) - 1); u1.num++) {
		vo_p0 += negative_chunk_delta;

		// Inconsistent parentheses? Blame code generation.
		dots = egc_chunk(vo_p0 + CHUNK_OFFSET_LEFT);
		egc_chunk((vo_p0 - negative_chunk_delta + CHUNK_OFFSET_LEFT)) = dots;
		dots = egc_chunk(vo_p0 + CHUNK_OFFSET_RIGHT);
		egc_chunk((vo_p0 - negative_chunk_delta + CHUNK_OFFSET_RIGHT)) = dots;
	}

	// Transfer new pixels
	u1.vo_p1 = ((negative_chunk_delta > 0)
		? ((y * ROW_SIZE) + transferred_offset)
		: ((y * ROW_SIZE) + ROW_SIZE - ROWSHIFT_CHUNK_SIZE - transferred_offset)
	);
	page_access(1);	dots = egc_chunk(u1.vo_p1 + CHUNK_OFFSET_LEFT);
	page_access(0);	egc_chunk(vo_p0 + CHUNK_OFFSET_LEFT) = dots;
	page_access(1);	dots = egc_chunk(u1.vo_p1 + CHUNK_OFFSET_RIGHT);
	page_access(0);	egc_chunk(vo_p0 + CHUNK_OFFSET_RIGHT) = dots;
}

#include "th01/main/totle_transition.inc"

void near totle_load_and_pagetrans_animate(void)
{
	page_access(1);
	grp_put("CLEAR3.grp", GPF_PALETTE_KEEP);
	ptn_load(PTN_SLOT_NUMB, "numb.ptn");
	page_access(0);
	totle_pagetrans_animate(0);
}

inline void totle_free(void) {
	ptn_free(PTN_SLOT_NUMB);
}

void near stagebonus_box_open_animate(void)
{
	vram_offset_t vo_upper_row = vram_offset_shift(
		STAGEBONUS_PADDED_LEFT,
		(STAGEBONUS_PADDED_TOP + (STAGEBONUS_PADDED_H / 2))
	);
	pixel_t x;
	vram_offset_t vo_upper_column;
	vram_offset_t vo_lower_row;
	vram_offset_t vo_lower_column;
	const dot_rect_t(8, 4) BOX = sSTAGEBONUS_BOX[0];

	grcg_setcolor_rmw(V_BLACK);

	vo_lower_row = vo_upper_row;
	for(pixel_t row = 0; row < (STAGEBONUS_PADDED_H / 2); row++) {
		x = 0;
		vo_upper_column = vo_upper_row;
		vo_lower_column = vo_lower_row;
		while(x < (STAGEBONUS_PADDED_W / BYTE_DOTS)) {
			grcg_put(vo_upper_column, BOX[row % BOX.h()], 8);
			grcg_put(vo_lower_column, BOX[row % BOX.h()], 8);
			x++;
			vo_upper_column++;
			vo_lower_column++;
		}
		vo_upper_row -= ROW_SIZE;
		vo_lower_row += ROW_SIZE;
		if((row % 6) == 0) {
			frame_delay(1);
		}
	}
	grcg_off_func();
}

static const int8_t FULLWIDTH_NUMERAL_SPACE = 10;

void fullwidth_numeral(ShiftJISKanji& kanji, int8_t digit)
{
	const shiftjis_t* NUMERALS[11] = FULLWIDTH_NUMERALS_WITH_SPACE;
	kanji.byte[0] = NUMERALS[digit][0];
	kanji.byte[1] = NUMERALS[digit][1];
}

void near fullwidth_str_from_4_digit_value(ShiftJISKanji str[4], int val)
{
	fullwidth_numeral(str[0], (val / 1000));	val %= 1000;
	fullwidth_numeral(str[1], (val / 100)); 	val %= 100;
	fullwidth_numeral(str[2], (val / 10));  	val %= 10;
	fullwidth_numeral(str[3], val);
}

#define stagebonus_str_put(relative_left, row, col, str) { \
	graph_putsa_fx( \
		(STAGEBONUS_PADDED_LEFT + GLYPH_FULL_W + relative_left), \
		(STAGEBONUS_PADDED_TOP + 40 + (row * GLYPH_H)), \
		(col | FX_WEIGHT_BOLD), \
		str \
	); \
}

void stagebonus_animate(int stage_num)
{
	enum {
		DIGITS = ((sizeof(stagebonus_digit_buf) - 1) / 2),
		METRIC_W = shiftjis_w(stagebonus_digit_buf),

		// Right-aligned?
		TITLE_LEFT = (STAGEBONUS_W - STAGEBONUS_TITLE_W),

		LABEL_LEFT = 0,

		// Technically reaching into the right padding.
		METRIC_LEFT = (STAGEBONUS_W + GLYPH_FULL_W - METRIC_W),

		TOTAL_LEFT = (STAGEBONUS_W - METRIC_W),
		HIT_KEY_LEFT = (
			STAGEBONUS_W - (2 * GLYPH_FULL_W) - shiftjis_w(STAGEBONUS_HIT_KEY)
		),
	};

	#define clamp_add_x10_to_score_bonus_and_put( \
		digit_buf, row, val, max, unnecessary_i \
	) { \
		val = clamp_and_add_x10_to_score_bonus(val, max, unnecessary_i); \
		fullwidth_str_from_4_digit_value(digit_buf, val); \
		stagebonus_str_put(METRIC_LEFT, row, V_WHITE, digit_buf->byte); \
	}

	stagebonus_title_t* title = reinterpret_cast<stagebonus_title_t *>(
		&stagebonus_title
	);
	ShiftJISKanji* digit_buf = reinterpret_cast<ShiftJISKanji *>(
		stagebonus_digit_buf
	);
	int val = 0;
	unsigned int i;

	fullwidth_numeral(title->stage[0], (stage_num / 10));
	fullwidth_numeral(title->stage[1], (stage_num % 10));

	z_palette_set_all_show(stage_palette);
	stagebonus_box_open_animate();

	stagebonus_str_put(TITLE_LEFT, 0, V_RED, title->prefix->byte);
	stagebonus_str_put(LABEL_LEFT, 2, V_WHITE, STAGEBONUS_SUBTITLE);
	stagebonus_str_put(LABEL_LEFT, 3, V_WHITE, STAGEBONUS_TIME);
	stagebonus_str_put(LABEL_LEFT, 5, V_WHITE, STAGEBONUS_CARDCOMBO_MAX);
	stagebonus_str_put(LABEL_LEFT, 7, V_WHITE, STAGEBONUS_RESOURCES);
	stagebonus_str_put(LABEL_LEFT, 9, V_WHITE, STAGEBONUS_STAGE_NUMBER);
	stagebonus_str_put(LABEL_LEFT, 11, V_WHITE, STAGEBONUS_TOTAL);
	stagebonus_str_put(HIT_KEY_LEFT, 13, V_RED, STAGEBONUS_HIT_KEY);

	// Time
	val = stagebonus_time_value(stage_timer);
	val = clamp_and_add_x10_to_score_bonus(val, 6553, i);
	fullwidth_numeral(digit_buf[4], 0);
	fullwidth_numeral(digit_buf[5], FULLWIDTH_NUMERAL_SPACE);
	fullwidth_str_from_4_digit_value(digit_buf, val);
	stagebonus_str_put(METRIC_LEFT, 3, V_WHITE, digit_buf->byte);

	// Card combo ("Continuous")
	val = stagebonus_combo_value(cardcombo_max);
	clamp_add_x10_to_score_bonus_and_put(digit_buf, 5, val, 6553, i);

	// Bomb & Player
	val = stagebonus_resources_value(rem_lives, rem_bombs);
	clamp_add_x10_to_score_bonus_and_put(digit_buf, 7, val, 6553, i);

	// Stage number
	val = stagebonus_stage_value(stage_num);
	clamp_add_x10_to_score_bonus_and_put(digit_buf, 9, val, 6553, i);

	// Commit
	score += score_bonus;

	// Sum
	int bonus_div_10000 = (score_bonus / 10000);
	int bonus_remainder = (score_bonus - (bonus_div_10000 * 10000));

	resident->bonus_per_stage[(stage_num - 1) % STAGES_PER_SCENE] = score_bonus;

	fullwidth_numeral(digit_buf[0], (bonus_div_10000 / 10));
	fullwidth_numeral(digit_buf[1], (bonus_div_10000 % 10));
	fullwidth_numeral(digit_buf[2], (bonus_remainder / 1000));
	bonus_remainder %= 1000;
	fullwidth_numeral(digit_buf[3], (bonus_remainder / 100));
	bonus_remainder %= 100;
	fullwidth_numeral(digit_buf[4], (bonus_remainder / 10));
	fullwidth_numeral(digit_buf[5], 0);
	stagebonus_str_put(TOTAL_LEFT, 11, V_WHITE, digit_buf->byte);

	// Remember when we added to the [score] above?
	hud_score_and_cardcombo_render();

	// Don't you love explicitly spelling out everything that can happen?
	score_extend_update_and_render();

	input_reset_sense();
	input_shot = true;
	input_ok = true;
	while((input_shot == true) && (input_ok == true)) {
		input_sense(false);
	}

	score_bonus = 0;

	#undef clamp_add_x10_to_score_bonus_and_put
}

typedef long totle_value_t;
#include "th01/main/totle_animate.inc"
