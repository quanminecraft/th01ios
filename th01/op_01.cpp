/* ReC98
 * -----
 * Code segment #1 of TH01's OP.EXE
 */

#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include "platform/x86real/pc98/page.hpp"
#include "platform/x86real/pc98/keyboard.hpp"
#include "game/input.hpp"
#include "th01/math/clamp.hpp"
#include "th01/core/initexit.hpp"
#include "th01/hardware/egc.h"
#include "th01/hardware/frmdelay.h"
#include "th01/hardware/graph.h"
#include "th01/hardware/grppsafx.h"
#include "th01/hardware/grp_text.hpp"
#include "th01/hardware/input.hpp"
#include "th01/hardware/palette.h"
#include "th01/formats/cfg.hpp"
#include "th01/formats/grp.h"
#include "th01/snd/mdrv2.h"
#include "th01/shiftjis/debug.hpp"
#include "th01/shiftjis/fns.hpp"
#include "th01/shiftjis/op.hpp"

/// REIIDEN.CFG loading and saving
/// ------------------------------

cfg_options_t opts = {
	CFG_RANK_DEFAULT,
	CFG_BGM_MODE_DEFAULT,
	CFG_CREDIT_BOMBS_DEFAULT,
	CFG_CREDIT_LIVES_EXTRA_DEFAULT
};
int8_t debug_mode = 0;

void cfg_load(void)
{
	cfg_t cfg_in;
	bool read_failure = false;

	if(!file_ropen(CFG_FN)) {
use_defaults:
		read_failure = true;
	}
	if(!read_failure) {
		file_read(&cfg_in, sizeof(cfg_in));
		if(memcmp(cfg_in.id, CFG_ID, sizeof(cfg_in.id))) {
			file_close();
			goto use_defaults;
		}
		opts.rank = cfg_in.opts.rank;
		opts.bgm_mode = cfg_in.opts.bgm_mode;
		opts.credit_bombs = cfg_in.opts.credit_bombs;
		opts.credit_lives_extra = cfg_in.opts.credit_lives_extra;
		file_close();
	} else {
		opts.rank = CFG_RANK_DEFAULT;
		opts.bgm_mode = CFG_BGM_MODE_DEFAULT;
		opts.credit_bombs = CFG_CREDIT_BOMBS_DEFAULT;
		opts.credit_lives_extra = CFG_CREDIT_LIVES_EXTRA_DEFAULT;
	}
}

void cfg_save(void)
{
	if(!file_create(CFG_FN)) {
		return;
	}
	file_write(CFG_ID, sizeof(CFG_ID) - 1);
	file_write(&opts, sizeof(opts));
	file_close();
}
/// ------------------------------

/// Input
/// -----

#define OP_INPUT_PREV static bool16 input_prev[2] = { false }
#define OP_LOCAL(type, name, value) static type name = value
#define OP_BGM_REF static_cast<int8_t>(opts.bgm_mode)

#include "th01/title_input.inc"

#include "th01/title_graphics.inc"

// Starting the game
// -----------------

void start_game(bool new_game)
{
	cfg_save();

	// If we went through main_setup(), we already allocated a resident
	// structure, but not it someone launched us via `debloat op` from GAME.BAT
	// (which you shouldn't do, but still makes sense to support for parity
	// reasons).
	resident = resident_get_or_create();

	resident->rank = opts.rank;
	resident->bgm_mode = opts.bgm_mode;
	resident->rem_bombs = opts.credit_bombs;
	resident->credit_lives_extra = opts.credit_lives_extra;
	resident->rand = frame_rand;
	resident->score = 0;
	key_end();
	mdrv2_bgm_fade_out_nonblock();

	// ZUN bug: Does not initialize [resident->debug_mode] if [debug_mode] is
	// 1, which happens when starting the game with `game s`. This mode is
	// supposed to just show the stage selection screen at the beginning of
	// REIIDEN.EXE, without enabling other debug features.
	// However, since master.lib doesn't clear the resident structure after
	// allocation, `game s` might actually *have* the intended effect if
	// anything on the system previously wrote a value other than DM_OFF,
	// DM_TEST, or DM_FULL to the specific byte in memory that now corresponds
	// to [resident->debug_mode]. This happens on a certain widely circulated
	// .HDI copy of TH01 that boots MS-DOS 3.30C and then directly launches the
	// game, which might be the whole reason why `game s` is widely documented
	// to trigger a stage selection to begin with.
	//
	// The fact that main() does have a conditional branch that sets
	// [debug_mode] to a distinct value proves that ZUN did have a stage
	// selection mode during development, fitting the bug classification.
	if(!new_game || (debug_mode == 0)) {
		resident->debug_mode = DM_OFF;
	} else if(debug_mode == 2) {
		resident->debug_mode = DM_TEST;
	} else if(debug_mode == 3) {
		resident->debug_mode = DM_FULL;
	}

	resident->rem_lives = (opts.credit_lives_extra + 2);
	resident->point_value = 0;

	if(new_game) {
		resident->route = ROUTE_MAKAI;
		resident->stage_id = 0;
		for(int i = 0; i < SCENE_COUNT; i++) {
			resident->continues_per_scene[i] = 0;
			resident->bonus_per_stage[i] = 0;
		}
		resident->score_highest = 0;
		resident->end_flag = ES_NONE;
	}

	resident->snd_need_init = true;
	resident->pellet_speed = to_pellet_speed(-0.1);

	game_switch_binary(EP_MAIN);
}
// -----------------

#include "th01/title_menu.inc"

int main_op(int argc, const char *argv[])
{
	int bgm_mode_cur = 0;
	int hit_key_frame = 0;
	const char* arg2; // ZUN bloat
	const char* arg4; // ZUN bloat
	uint8_t bios_flag;
	seg_t kb_buf_head;
	REGS in;
	REGS out;

	if(!mdrv2_resident()) {
		printf(ERROR_RESIDENT_INVALID);
		return 1;
	}

	unused_con_arg_0 = 0;
	unused_con_arg_1 = 0;
	unused_con_arg_2 = 0;

	if(argc > 1) {
		if(argv[1][0] == 's') {
			debug_mode = 1;
		}
		if(argv[1][0] == 't') {
			debug_mode = 2;
		}
		if(argv[1][0] == 'd') {
			debug_mode = 3;
		}

		// ZUN landmine: The string could theoretically be less than 3
		// characters long.
		if(memcmp(argv[1], "CON", 3) == 0) {
			arg2 = argv[2];
			unused_con_arg_0 = atol(arg2);
			unused_con_arg_1 = atol(argv[3]);
			arg4 = argv[4];
			unused_con_arg_2 = atol(arg4);
		}
	}

	// ZUN bug: Should be done conditionally based on resident_t::bgm_mode.
	// This is why you still get music during this binary despite disabling BGM
	// in the options.
	mdrv2_enable_if_board_installed();
	game_init();
	cfg_load();

	bgm_mode_cur = opts.bgm_mode;

	// Initialize the PC-98 keyboard interface and clear the key buffer.
	in.h.ah = 0x03;
	int86(0x18, &in, &out);

	key_start();

	title_init();

	// Set the BIOS_FLAG to not beep when the keyboard buffer overflows, as
	// we're clearing and setting its length to 0 below… I guess?
	pokeb(0, 0x0500 /* BIOS_FLAG */, (peekb(0, 0x0500 /* BIOS_FLAG */) | 0x20));

	while(!key_sense_bios()) {
		frame_delay(1);
		title_hit_key_put(hit_key_frame);
		hit_key_frame++;
	}

	title_window_put();

	// Since [frame_rand] is always 0 here, the white line animation always
	// looks identical.
	irand_init(frame_rand);

	while(!quit) {
		if(menu_id == MID_MAIN) {
			main_input_sense();
			main_update_and_render();
		} else if(menu_id == MID_OPTION) {
			option_input_sense();
			option_update_and_render();
		} else if(menu_id == MID_MUSIC) {
			option_input_sense();
			music_update_and_render();
		} else if(menu_id == MID_UPDATE_BGM_MODE__DELAY__SWITCH_TO_MAIN) {
			if(opts.bgm_mode != bgm_mode_cur) {
				if(opts.bgm_mode == BGM_MODE_OFF) {
					mdrv2_bgm_stop();
				} else if(static_cast<int>(opts.bgm_mode) == BGM_MODE_MDRV2) {
					mdrv2_bgm_stop();
					mdrv2_bgm_load("reimu.mdt");
					mdrv2_bgm_play();
				}
				bgm_mode_cur = opts.bgm_mode;
			}

			// ZUN quirk: These delays are only here to prevent this menu from
			// being immediately entered again if the OK or Shot key isn't
			// immediately released on the next frame. This would need a
			// cross-menu locking approach to be solved properly.
			frame_delay(15);
			menu_id = MID_MAIN;
		} else if(menu_id == MID_DELAY__SWITCH_TO_OPTION) {
			// ZUN quirk: Same here.
			frame_delay(15);
			menu_id = MID_OPTION;
		}

		// Clear the PC-98 BIOS keyboard buffer… yeah, this is not a proper
		// way to handle cross-menu locking either.
		poke(0, 0x0526 /* KB_BUF_TAIL */, peek(0, 0x0524) /* KB_BUF_HEAD */);
		pokeb(0, 0x0528 /* KB_COUNT */, 0);

		frame_rand++;
		frame_delay(1);
	}
	cfg_save();
	mdrv2_bgm_stop();
	key_end();

	page_access(1);	z_graph_clear();
	page_access(0);	z_graph_clear();

	game_exit();
	mdrv2_bgm_stop();
	printf(GOODBYE);

	return 0;
}
/// ---------
