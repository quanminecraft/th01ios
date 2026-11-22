/// Jigoku Stage 20 Boss - Konngara
/// -------------------------------

#ifdef TH01_NATIVE_ORB_PHYSICS
#include "native/konngara_backend.hpp"
namespace konngara_original {
#else
#include <stdio.h>
#include "libs/master.lib/pc98_gfx.hpp"
#include "th01/rank.h"
#include "th01/resident.hpp"
#include "th01/v_colors.hpp"
#include "th01/math/overlap.hpp"
#include "th01/math/vector.hpp"
#include "th01/hardware/frmdelay.h"
#include "th01/hardware/palette.h"
#include "th01/hardware/graph.h"
#include "th01/hardware/scrollup.hpp"
#include "th01/hardware/text.h"
#include "th01/snd/mdrv2.h"
#include "th01/formats/grp.h"
#include "th01/formats/grz.h"
#include "th01/sprites/pellet.h"
#include "th01/sprites/shape8x8.hpp"
#include "th01/main/spawnray.hpp"
#include "th01/main/stage/palette.hpp"
#include "th01/main/stage/stageobj.hpp"
#include "th01/main/stage/stages.hpp"
#include "th01/main/shape.hpp"
#include "th01/main/player/player.hpp"
#include "th01/main/player/shot.hpp"
#include "th01/main/boss/entity_a.hpp"
#include "th01/main/boss/palette.hpp"
#include "th01/main/bullet/pellet.hpp"
#include "th01/main/bullet/laser_s.hpp"
#include "th01/main/hud/hp.hpp"
#include "platform/x86real/pc98/egc.hpp"
#include "platform/x86real/pc98/page.hpp"
typedef int konngara_int_t;
#endif

static const char* unused_entrance_letters_maybe[] = { "ANGEL", "OF", "DEATH" };

// Coordinates
// -----------

static const screen_x_t HEAD_LEFT = 280;
static const screen_y_t HEAD_TOP = 80;
static const screen_x_t FACE_LEFT = 280;
static const screen_y_t FACE_TOP = 128;
static const screen_x_t CUP_LEFT = 296;
static const screen_y_t CUP_TOP = 188;
static const screen_x_t LEFT_SLEEVE_LEFT = 290;
static const screen_y_t LEFT_SLEEVE_TOP = 200;
static const screen_x_t EYE_CENTER_X = 316;
static const screen_y_t EYE_BOTTOM = 140;

static const screen_x_t HITBOX_LEFT = 288;
static const screen_y_t HITBOX_TOP = 120;

// Including the respective sprites:
//
// 	               [w]
// 	    ┌───────────────────────┐
// 	    │                       │
// 	[h] │                       │
// 	    │             sprite → ┌┤
// 	    └──────────────────────┴┘
//
static const pixel_t HITBOX_SHOT_W = 112;
static const pixel_t HITBOX_SHOT_H = 56;
static const pixel_t HITBOX_ORB_W = 96;
static const pixel_t HITBOX_ORB_H = 72;

// Slash pattern spawners are moved on a triangle along these points.
static const screen_x_t SWORD_CENTER_X = 410;
static const screen_y_t SWORD_CENTER_Y = 70;
static const screen_x_t SLASH_4_CORNER_X = 432;
static const screen_y_t SLASH_4_CORNER_Y = 232;
static const screen_x_t SLASH_5_CORNER_X = 198;
static const screen_y_t SLASH_5_CORNER_Y = 198;

static const pixel_t CUP_W = 32;

static const screen_x_t CUP_RIGHT = (CUP_LEFT + CUP_W);
static const screen_x_t CUP_CENTER_X = (CUP_LEFT + (CUP_W / 2));
// -----------

// Other constants
// ----------------

static const pixel_t SLASH_DISTANCE_2_TO_4_X = (
	SLASH_4_CORNER_X - SWORD_CENTER_X
);
static const pixel_t SLASH_DISTANCE_2_TO_4_Y = (
	SLASH_4_CORNER_Y - SWORD_CENTER_Y
);
static const pixel_t SLASH_DISTANCE_5_TO_4_X = (
	SLASH_5_CORNER_X - SLASH_4_CORNER_X // yes, backwards
);
static const pixel_t SLASH_DISTANCE_5_TO_4_Y = (
	SLASH_5_CORNER_Y - SLASH_4_CORNER_Y // yes, backwards
);
static const pixel_t SLASH_DISTANCE_5_TO_6_X = (
	SLASH_5_CORNER_X - SWORD_CENTER_X
);
static const pixel_t SLASH_DISTANCE_5_TO_6_Y = (
	SLASH_5_CORNER_Y - SWORD_CENTER_Y
);
// ----------------

static union {
	konngara_int_t group; // pellet_group_t
	konngara_int_t interval;
	subpixel_t speed;
	pixel_t delta_x;
	konngara_int_t unused;
} pattern_state;

// Entities
// --------

enum face_direction_t {
	FD_RIGHT = 0,
	FD_LEFT = 1,
	FD_CENTER = 2,
	FD_COUNT = 3,
	FD_UNINITIALIZED = 9, // :zunpet:

	_face_direction_t_FORCE_INT16 = 0x7FFF
};

enum face_expression_t {
	FE_NEUTRAL = 0,
	FE_CLOSED = 1,
	FE_GLARE = 2,
	FE_AIM = 3,

	_face_expression_t_FORCE_INT16 = 0x7FFF
};

static face_direction_t face_direction = FD_UNINITIALIZED;
static face_expression_t face_expression = FE_NEUTRAL;
static bool16 face_direction_can_change = true;

#define ent_head                	boss_entity_0
#define ent_face_closed_or_glare	boss_entity_1
#define ent_face_aim            	boss_entity_2

inline void konngara_ent_load(void) {
	ent_head.load("boss8_1.bos", 0);
	ent_face_closed_or_glare.load("boss8_e1.bos", 1);
	ent_face_aim.load("boss8_e2.bos", 2);
}

inline void konngara_ent_free(void) {
	bos_entity_free(0);
	bos_entity_free(1);
	bos_entity_free(2);
}

#define head_put(direction) \
	ent_head.put_8(HEAD_LEFT, HEAD_TOP, direction);

#define face_aim_put(direction) \
	ent_face_aim.put_8(FACE_LEFT, FACE_TOP, direction);

#define face_put(expression, direction) \
	ent_face_closed_or_glare.put_8( \
		FACE_LEFT, FACE_TOP, (((expression - FE_CLOSED) * FD_COUNT) + direction) \
	);
// --------

// Snakes
// ------

#include "th01/main/boss/konngara_snakes_common.inc"

// File names
// ----------

#define SCROLL_BG_FN "boss7_d1.grp"
#ifndef TH01_NATIVE_ORB_PHYSICS
#include "th01/shiftjis/fns.hpp"
#endif
#define GRZ_FN "boss8.grz"
// ----------

#ifndef TH01_NATIVE_ORB_PHYSICS
#include "th01/main/select_r.cpp"
#endif

// Almost identical to Sariel's version. This one is better.
static void spawnray_unput_and_put(
	screen_x_t origin_x, vram_y_t origin_y,
	screen_x_t target_x, vram_y_t target_y,
	vc2 col
)
{
	static screen_x_t target_prev_x = -PIXEL_NONE;
	static vram_y_t target_prev_y = -PIXEL_NONE;
	spawnray_unput_and_put_func(
		target_prev_x, target_prev_y,
		origin_x, origin_y, target_x, target_y, col, false
	);
}

// Siddhaṃ seed syllables
// ----------------------

#define SIDDHAM_COL 0x9

inline void siddham_col_white(void) {
	z_palette_set_show(SIDDHAM_COL, 0xF, 0xF, 0xF);
}

#define siddham_col_white_in_step() \
	if(z_Palettes[SIDDHAM_COL].c.r > 0x0) { \
		z_Palettes[SIDDHAM_COL].c.r--; \
	} \
	if(z_Palettes[SIDDHAM_COL].c.g > 0x9) { \
		z_Palettes[SIDDHAM_COL].c.g--; \
	} \
	if(z_Palettes[SIDDHAM_COL].c.b > 0xA) { \
		z_Palettes[SIDDHAM_COL].c.b--; \
	} \
	z_palette_set_all_show(z_Palettes);
// ----------------------

#ifdef TH01_NATIVE_ORB_PHYSICS
void konngara_load_and_entrance(int8_t) {native_konngara_intro();konngara_ent_load();}
#else
#include "th01/main/boss/konngara_entrance.inc"
#endif

void konngara_init(void)
{
	boss_palette_snap();
	void konngara_setup();
	konngara_setup();
}

void konngara_setup(void)
{
	boss_hp = 18;
	hud_hp_first_white = 16;
	hud_hp_first_redwhite = 10;
	boss_phase = 0;
	boss_phase_frame = 0;
	face_direction_can_change = true;
	face_expression = FE_NEUTRAL;
	face_direction = FD_CENTER;
}

// Happens to be entirely protected from double frees. Yes, this matters.
void konngara_free(void)
{
	konngara_ent_free();
	for(int i = 0; i < 7; i++) {
		grx_free(i);
	}
}

#include "th01/main/boss/konngara_face.inc"

void slash_put(int image)
{
	page_access(1);	grx_put(image);
	page_access(0);	grx_put(image);
}

#include "th01/main/boss/konngara_early.inc"

#include "th01/main/boss/konngara_two_snakes.inc"

#include "th01/main/boss/konngara_rows_spray.inc"

#include "th01/main/boss/konngara_four_snakes.inc"

#include "th01/main/boss/konngara_swordray.inc"

#include "th01/main/boss/konngara_slash.inc"

#include "th01/main/boss/konngara_lasers.inc"

#include "th01/main/boss/konngara_slash_aimed.inc"

#include "th01/main/boss/konngara_sleeve.inc"

#ifndef TH01_NATIVE_ORB_PHYSICS
#include "th01/main/boss/konngara_kuji.inc"
#endif

void konngara_main(void)
{
#ifdef TH01_NATIVE_ORB_PHYSICS
    if(!native_konngara_before_frame()) return;
#endif
	static struct {
		bool16 invincible;
		konngara_int_t invincibility_frame;

		void update_and_render(const vc_t (&flash_colors)[3]) {
			boss_hit_update_and_render(
				invincibility_frame,
				invincible,
				boss_hp,
				flash_colors,
				(sizeof(flash_colors) / sizeof(flash_colors[0])), \
				10000,
				overlap_xy_xywh_le_ge_2(
					orb_cur_left,
					orb_cur_top,
					HITBOX_LEFT,
					HITBOX_TOP,
					(HITBOX_ORB_W - ORB_W),
					(HITBOX_ORB_H - ORB_H)
				),
				shot_hitbox_t(
					HITBOX_LEFT, HITBOX_TOP, HITBOX_SHOT_W, HITBOX_SHOT_H
				)
			);
		}
	} hit;

	enum {
		CHOOSE_NEW = 99,
	};

	// The IDs are associated with a different pattern in every phase.
	static konngara_int_t pattern_prev;
	static bool initial_hp_rendered;

	static struct {
		konngara_int_t pattern_cur;
		konngara_int_t patterns_done;

		void next(int8_t phase_new) {
			boss_phase = phase_new;
			boss_phase_frame = 0;
			hit.invincibility_frame = 0;
			pattern_cur = CHOOSE_NEW;
			patterns_done = 0;
			if(phase_new & 1) {
				face_expression_set_and_put(FE_NEUTRAL);
			}
		}

		void frame_common(face_direction_t& fd_new) {
			boss_phase_frame++;
			hit.invincibility_frame++;

			fd_new =
				(player_left < 198) ? FD_LEFT :
				(player_left > 396) ? FD_RIGHT : FD_CENTER;
			face_direction_set_and_put(fd_new);
		}
	} phase = { 0, 0 };

	int i;
	int j;
	svc2 col;
	int comp;
	int scroll_frame;
	face_direction_t fd_track;
	const vc_t flash_colors[3] = { 3, 4, 5 };

	#define pattern_choose( \
		phase, frame_min, count_on_first_try, count_on_second_try \
	) { \
		if(boss_phase_frame > frame_min) { \
			boss_phase_frame = 1; \
			phase.pattern_cur = (irand() % count_on_first_try); \
			if(phase.pattern_cur == pattern_prev) { \
				phase.pattern_cur = (irand() % count_on_second_try); \
			} \
			pattern_prev = phase.pattern_cur; \
			phase.patterns_done++; \
		} \
	}

	#define phase_frame_siddham_flash(phase, next_phase) { \
		if(boss_phase_frame == 50) { \
			siddham_col_white(); \
		} \
		if((boss_phase_frame > 50) && ((boss_phase_frame % 4) == 0)) { \
			siddham_col_white_in_step(); \
		} \
		\
		hit.update_and_render(flash_colors); \
		if(!hit.invincible && (boss_phase_frame > 120)) { \
			phase.next(next_phase); \
		} \
	}

	if(boss_phase == 0) {
		boss_phase = 1;
		pattern_prev = CHOOSE_NEW;
		phase.pattern_cur = CHOOSE_NEW;
		phase.patterns_done = 0;
		boss_phase_frame = 0;
		hit.invincibility_frame = 0;
		hit.invincible = false;
		initial_hp_rendered = false;
		boss_palette_snap();
		random_seed = frame_rand;
	} else if(boss_phase == 1) {
		// ZUN bug: Since the fight only ends in Phase 7 at the earliest, HP
		// subtraction in debug mode can lead to this function being called
		// with [boss_phase_frame] being larger than the initial HP value,
		// causing the heap corruption bug mentioned in the comment of this
		// function. Konngara starts with an even number of total HP, so this
		// will even happen for the easiest possible case of holding ↵ Return
		// for the first 10 frames of phase 1.
		hud_hp_increment_render(initial_hp_rendered, boss_hp, boss_phase_frame);

		phase.frame_common(fd_track);

		if(phase.pattern_cur == 0) {
			pattern_diamond_cross_to_edges_followed_by_rain();
		} else if(phase.pattern_cur == 1) {
			pattern_symmetrical_from_cup();
		} else if(phase.pattern_cur == 2) {
			pattern_two_homing_snakes_and_semicircle_spreads();
		} else if(phase.pattern_cur == CHOOSE_NEW) {
			pattern_choose(phase, 120, 3, 3);
		}
		if(boss_phase_frame == 0) {
			face_expression_set_and_put(FE_NEUTRAL);
			phase.pattern_cur = CHOOSE_NEW;
		}
		hit.update_and_render(flash_colors);
		if(!hit.invincible && ((phase.patterns_done >= 7) || (boss_hp < 16))) {
			if(phase.pattern_cur == CHOOSE_NEW) {
				phase.next(2);
			}
		}
	} else if(boss_phase == 2) {
		phase.frame_common(fd_track);
		phase_frame_siddham_flash(phase, 3);
	} else if(boss_phase == 3) {
		phase.frame_common(fd_track);

		if(phase.pattern_cur == 0) {
			pattern_aimed_rows_from_top();
		} else if(phase.pattern_cur == 1) {
			pattern_aimed_spray_from_cup();
		} else if(phase.pattern_cur == 2) {
			pattern_four_homing_snakes();
		} else if(phase.pattern_cur == 3) {
			pattern_rain_from_edges();
		} else if(phase.pattern_cur == CHOOSE_NEW) {
			pattern_choose(phase, 120, 4, 4);
		}

		if(boss_phase_frame == 0) {
			face_expression_set_and_put(FE_NEUTRAL);
			phase.pattern_cur = CHOOSE_NEW;
		}

		hit.update_and_render(flash_colors);
		if(!hit.invincible && ((phase.patterns_done >= 9) || (boss_hp < 13))) {
			if(phase.pattern_cur == CHOOSE_NEW) {
				phase.next(4);
			}
		}
	} else if(boss_phase == 4) {
		phase.frame_common(fd_track);
		phase_frame_siddham_flash(phase, 5);
	} else if(boss_phase == 5) {
		phase.frame_common(fd_track);

		if(phase.pattern_cur == 0) {
			pattern_slash_rain();
		} else if(phase.pattern_cur == 1) {
			pattern_lasers_and_3_spread();
		} else if(phase.pattern_cur == 2) {
			pattern_slash_triangular();
		} else if(phase.pattern_cur == CHOOSE_NEW) {
			pattern_choose(phase, 120, 3, 2);
		}

		if(boss_phase_frame == 0) {
			face_expression_set_and_put(FE_NEUTRAL);
			phase.pattern_cur = CHOOSE_NEW;
		}

		hit.update_and_render(flash_colors);
		if(!hit.invincible && ((phase.patterns_done >= 6) || (boss_hp < 10))) {
			if(phase.pattern_cur == CHOOSE_NEW) {
				phase.next(6);
			}
		}
	} else if(boss_phase == 6) {
		phase.frame_common(fd_track);
		phase_frame_siddham_flash(phase, 7);
	} else if(boss_phase == 7) {
		phase.frame_common(fd_track);

		if(phase.pattern_cur == 0) {
			pattern_diamond_cross_to_edges_followed_by_rain();
		} else if(phase.pattern_cur == 1) {
			pattern_symmetrical_from_cup();
		} else if(phase.pattern_cur == 2) {
			pattern_two_homing_snakes_and_semicircle_spreads();
		} else if(phase.pattern_cur == 3) {
			pattern_aimed_rows_from_top();
		} else if(phase.pattern_cur == 4) {
			pattern_aimed_spray_from_cup();
		} else if(phase.pattern_cur == 5) {
			pattern_four_homing_snakes();
		} else if(phase.pattern_cur == 6) {
			pattern_rain_from_edges();
		} else if(phase.pattern_cur == 7) {
			pattern_slash_rain();
		} else if(phase.pattern_cur == 8) {
			pattern_lasers_and_3_spread();
		} else if(phase.pattern_cur == 9) {
			pattern_slash_triangular();
		} else if(phase.pattern_cur == 10) {
			pattern_semicircle_rain_from_sleeve();
		} else if(phase.pattern_cur == 11) {
			pattern_slash_aimed();
		} else if(phase.pattern_cur == CHOOSE_NEW) {
			pattern_choose(phase, 5, 12, 2);
		}

		if(boss_phase_frame == 0) {
			face_expression_set_and_put(FE_NEUTRAL);
			phase.pattern_cur = CHOOSE_NEW;
		}

		hit.update_and_render(flash_colors);
		#ifdef TH01_NATIVE_ORB_PHYSICS
        native_konngara_observe(phase.pattern_cur,phase.patterns_done);
#endif
        if(boss_hp > 0) {
			return;
		}

		// Defeat sequence (blocking)
		// ==========================

		#ifdef TH01_NATIVE_ORB_PHYSICS
        native_konngara_defeat();
#else
#include "th01/main/boss/konngara_defeat.inc"
#endif
	}

#ifdef TH01_NATIVE_ORB_PHYSICS
    native_konngara_observe(phase.pattern_cur,phase.patterns_done);
#endif
	#undef phase_frame_siddham_flash
	#undef pattern_choose
}

#ifdef TH01_NATIVE_ORB_PHYSICS
}
#endif
