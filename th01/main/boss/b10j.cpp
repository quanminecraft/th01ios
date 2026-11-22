/// Jigoku Stage 10 Boss - Mima
/// ---------------------------

#ifdef TH01_NATIVE_ORB_PHYSICS
#include "native/mima_backend.hpp"
namespace mima_original {
#else
#include "platform/x86real/pc98/page.hpp"
#include "th01/rank.h"
#include "th01/resident.hpp"
#include "th01/v_colors.hpp"
#include "th01/math/dir.hpp"
#include "th01/math/polar.hpp"
#include "th01/math/vector.hpp"
#include "th01/hardware/frmdelay.h"
#include "th01/hardware/graph.h"
#include "th01/hardware/palette.h"
#include "th01/snd/mdrv2.h"
#include "th01/sprites/pellet.h"
#include "th01/sprites/pillar.hpp"
#include "th01/main/particle.hpp"
#include "th01/main/shape.hpp"
#include "th01/main/boss/defeat.hpp"
#include "th01/main/boss/entity_a.hpp"
#include "th01/main/bullet/laser_s.hpp"
#include "th01/main/bullet/missile.hpp"
#include "th01/main/bullet/pellet.hpp"
#include "th01/main/hud/hp.hpp"
#include "th01/main/player/player.hpp"
#include "th01/main/player/shot.hpp"
#include "th01/main/stage/palette.hpp"
typedef int mima_int_t;
#endif

// Coordinates
// -----------

static const pixel_t MIMA_W = 128;
static const pixel_t MIMA_H = 160;

// Including the sprite:
//
// 	               [w]
// 	    ┌───────────────────────┐
// 	    │                       │
// 	[h] │                       │
// 	    │             sprite → ┌┤
// 	    └──────────────────────┴┘
static const pixel_t HITBOX_SHOT_W = (MIMA_W - (MIMA_W / 8));
static const pixel_t HITBOX_SHOT_H = ((MIMA_H * 2) / 5);

static const pixel_t MIMA_ANIM_TOP = 48; // relative to the sprite's top edge
static const pixel_t MIMA_ANIM_H = 64;

static const screen_x_t BASE_CENTER_X = PLAYFIELD_CENTER_X;
static const screen_y_t BASE_CENTER_Y = (
	PLAYFIELD_TOP + ((PLAYFIELD_H / 42) * 17)
);

static const screen_x_t BASE_LEFT = (BASE_CENTER_X - (MIMA_W / 2));
static const screen_y_t BASE_TOP = (BASE_CENTER_Y - (MIMA_H / 2));

// Not quite matching the image, but close enough.
static const pixel_t SEAL_RADIUS = 80;

// The radius of the circumscribed square around a circle with radius 𝓇 is
// 𝓇 * √2; see https://www.desmos.com/calculator/u8mtn9y9wo.
static const pixel_t SEAL_CIRCUMSQUARE_RADIUS = (SEAL_RADIUS * 1.41f);
// -----------

enum mima_colors_t {
	COL_SPAWNRAY = 1,
	COL_PILLAR = V_RED,
};

// Always denotes the last phase that ends with that amount of HP.
enum mima_hp_t {
	HP_TOTAL = 12,
	HP_PHASE_1_END = 6,
	HP_PHASE_3_END = 0,
};

// State
// -----

// Whether meteor_put() has any effect.
bool meteor_active = true;

// Amount of frames between the individual steps of the spread-in transition
uint8_t spreadin_interval = 4;

// Sprite pixels to spread in per frame, in one half of Mima's sprite
pixel_length_8_t spreadin_speed = 8;
// -----

// Entities
// --------

static const int METEOR_CELS = 4;

enum anim_cel_t {
	C_CAST = 0,
	C_METEOR = 1,
	C_METEOR_last = (C_METEOR + METEOR_CELS - 1),
};

#ifdef TH01_NATIVE_ORB_PHYSICS
static CBossEntitySized<MIMA_W,MIMA_H> native_mima_entity;
#define ent_still native_mima_entity
#else
#define ent_still \
	reinterpret_cast<CBossEntitySized<MIMA_W, MIMA_H> &>(boss_entity_0)
#endif

#define ent_anim 	boss_entity_1

inline void mima_ent_load(void) {
	ent_still.load("boss3_1.bos", 0);
	ent_anim.load("boss3_2.bos", 1);
}

inline void mima_ent_free(void) {
	bos_entity_free(0);
	bos_entity_free(1);
}
// --------

// .PTN
// ----

static const main_ptn_slot_t PTN_SLOT_BG_ENT = PTN_SLOT_BOSS_1;
static const main_ptn_slot_t PTN_SLOT_MISSILE = PTN_SLOT_BOSS_2;

// ZUN bloat: Three unused background .PTN IDs, for three unused 32×32
// animations?
static const int BG_ENT_OFFSET = 3;
// ----

// Patterns
// --------

static union {
	subpixel_t speed;
	DecimalSubpixel speed_decimal;
	mima_int_t speed_multiplied_by_8;
	mima_int_t unused;
} pattern_state;
// --------

void mima_load(void)
{
#ifdef TH01_NATIVE_ORB_PHYSICS
    native_mima_reset_backend();
#endif
	mima_ent_load();
	void mima_setup(void);
	mima_setup();
	ptn_new(
		PTN_SLOT_BG_ENT,
		(((MIMA_W / PTN_W) * (MIMA_H / PTN_H)) + BG_ENT_OFFSET + 1)
	);
	Missiles.load(PTN_SLOT_MISSILE);
	Missiles.reset();
}

inline void ent_anim_sync_with_still(void) {
	ent_anim.pos_cur_set(
		ent_still.cur_left, (ent_still.cur_top + MIMA_ANIM_TOP)
	);
}

inline void ent_anim_sync_with_still_and_put_both(int cel) {
	ent_anim_sync_with_still();
	ent_anim.set_image(cel);
	page_access(1);	ent_anim.put_8(cel);
	page_access(0);	ent_anim.put_8(cel);
}

void meteor_put(void)
{
	if(meteor_active && ((boss_phase_frame % 8) == 0)) {
		ent_anim_sync_with_still();
		ent_anim.set_image_unput_and_put_8(
			(C_METEOR + ((boss_phase_frame / 8) % METEOR_CELS))
		);
	}
}

void mima_put_cast_both(void)
{
	// ZUN bug: Does not unblit the meteor if `true`, and C_CAST does not
	// completely overlap any C_METEOR cel. In that case, small parts of the
	// meteor are guaranteed to be left in VRAM until they're unblitted as a
	// result of another sprite flying over them.
	meteor_active = false;

	ent_anim_sync_with_still_and_put_both(C_CAST);
}

void meteor_activate(void)
{
	if(!meteor_active) {
		meteor_active = true;
		ent_anim_sync_with_still_and_put_both(C_METEOR);
	}
}

void mima_put_still_both(void)
{
	page_access(1);	ent_still.put_8();
	page_access(0);	ent_still.put_8();
}

void mima_bg_snap(void)
{
	int ptn_x;
	int ptn_y;
	screen_x_t left = ent_still.cur_left;
	screen_y_t top = ent_still.cur_top;
	int image = BG_ENT_OFFSET;

	ptn_snap_rect_from_1_8(
		left, top, MIMA_W, MIMA_H, PTN_SLOT_BG_ENT, image, ptn_x, ptn_y
	);
}

void mima_unput(bool16 just_the_animated_part = false)
{
	int ptn_x;
	int image = BG_ENT_OFFSET;
	screen_x_t left = ent_still.cur_left;
	screen_y_t top = ent_still.cur_top;

	if(!just_the_animated_part) {
		int ptn_y;
		ptn_put_rect_noalpha_8(
			left, top, MIMA_W, MIMA_H, PTN_SLOT_BG_ENT, image, ptn_x, ptn_y
		);
		return;
	}

	// (The code below is never executed in the original game.)

	// Advance to the .PTN background row that contains the background behind
	// the animating part of Mima's sprite (i.e., the second one)
	image = (BG_ENT_OFFSET + ((MIMA_ANIM_TOP / PTN_H) * (MIMA_W / PTN_W)));

	// And since MIMA_ANIM_TOP is only a multiple of 16 and not 32, we have to
	// first awkwardly unblit a MIMA_W×16 area...
	for(ptn_x = 0; ptn_x < (MIMA_W / PTN_W); ptn_x++) {
		ptn_put_quarter_noalpha_8(
			(left + (0 * PTN_QUARTER_W) + (ptn_x * PTN_W)),
			(top + MIMA_ANIM_TOP),
			PTN_ID(PTN_SLOT_BG_ENT, image),
			(((MIMA_ANIM_TOP % PTN_H) / PTN_QUARTER_H) * 2)
		);
		ptn_put_quarter_noalpha_8(
			(left + (1 * PTN_QUARTER_W) + (ptn_x * PTN_W)),
			(top + MIMA_ANIM_TOP),
			PTN_ID(PTN_SLOT_BG_ENT, image),
			((((MIMA_ANIM_TOP % PTN_H) / PTN_QUARTER_H) * 2) + 1)
		);
		image++;
	}

	// ZUN landmine: Why is MIMA_ANIM_H assumed to be 48 (16 above + 32 here)?
	// This might have even worked if the bottom 16 pixels of all [ent_anim]
	// cels were identical, but they differ between C_CAST and C_METEOR.
	//
	// Note that this has nothing to do with Mima's infamous "third arm"
	// (remember, the game never executes this code), but wouldn't exactly
	// prevent it from happening either.
	#define bug_top (top + MIMA_ANIM_TOP + PTN_QUARTER_H)
	ptn_put_row_noalpha_8(left, bug_top, MIMA_W, PTN_SLOT_BG_ENT, image, ptn_x);
	#undef bug_top
}

inline void mima_unput_both(void) {
	page_access(1);	mima_unput();
	page_access(0);	mima_unput();
}

#include "th01/main/boss/mima_spreadin.inc"

// Only called while Mima isn't visible anyway. But even apart from that, it
// barely would have any effect anywhere, as the Mima sprite is blitted to both
// VRAM pages. This *might* have been supposed to crossfade between various
// cels? …Nah, why would you do that by blitting whole lines.
#include "th01/main/boss/mima_vertical_transition.inc"

void mima_setup(void)
{
	ent_still.set_image(0);
	ent_anim.set_image(C_METEOR);

	// We always come here with VRAM fully set to color #0 and #000 in that
	// hardware palette slot. In the original game, this call flashes from
	// white back to color #0 in boss3.grp, but for cleanup reasons, we haven't
	// loaded that palette on this branch yet. Thus, this call always flashes
	// to black, as ZUN intended.
	// (Even *if* you modded color #0 in that file, it would look very weird to
	// already see it at this point, before the entrance animation.)
	z_palette_white_in();

	ent_still.pos_set((PLAYFIELD_CENTER_X - (MIMA_W / 2)), PLAYFIELD_TOP);
	ent_still.hitbox_orb_set(
		((MIMA_W / 8) * 1), ((MIMA_H / 10) * 1),
		((MIMA_W / 8) * 7), ((MIMA_H / 10) * 9)
	);
	ent_still.hitbox_orb_inactive = false;
	boss_phase_frame = 0;
	boss_phase = 0;
	boss_hp = HP_TOTAL;
	hud_hp_first_white = HP_PHASE_1_END;
	hud_hp_first_redwhite = 2;  // fully arbitrary, doesn't indicate anything
	#ifdef TH01_NATIVE_ORB_PHYSICS
    Particles={};
#endif
	Particles.init();
}

void mima_free(void)
{
	mima_ent_free();
	ptn_free(PTN_SLOT_BG_ENT);
	ptn_free(PTN_SLOT_MISSILE);
}

#ifndef TH01_NATIVE_ORB_PHYSICS
#include "th01/main/select_r.cpp"
#endif

// Rotating square
// ---------------

#include "th01/main/boss/mima_square_patterns.inc"

#include "th01/main/boss/mima_hop.inc"

#ifdef TH01_NATIVE_ORB_PHYSICS
#include "native/pillar_sprites.inc"
#define sPILLAR native_pillar_rows
#else
#include "th01/sprites/pillar.csp"
#endif

#include "th01/main/boss/mima_pillars.inc"

#include "th01/main/boss/mima_square_late.inc"

void mima_main(void)
{
	#ifdef TH01_NATIVE_ORB_PHYSICS
    if(!native_mima_before_frame()) return;
#endif
	const vc_t flash_colors[2] = { 3, 9 };
	int i;

	static struct {
		mima_int_t invincibility_frame;
		bool16 invincible;

		void update_and_render(const vc_t (&flash_colors)[2]) {
			boss_hit_update_and_render(
				invincibility_frame,
				invincible,
				boss_hp,
				flash_colors,
				(sizeof(flash_colors) / sizeof(flash_colors[0])),
				5000,
				ent_still.hittest_orb(),
				shot_hitbox_t(
					(ent_still.cur_right() - HITBOX_SHOT_W),
					(ent_still.cur_top + (MIMA_H / 2) - (HITBOX_SHOT_H / 2)),
					HITBOX_SHOT_W,
					HITBOX_SHOT_H
				)
			);
		}
	} hit;
	static struct {
		int pattern_cur;

		void frame_common(void) const {
			boss_phase_frame++;
			hit.invincibility_frame++;
		}

		void pattern_next(int total) {
			pattern_cur = (pattern_cur == (total - 1)) ? 0 : (pattern_cur + 1);
		}

		void next(int phase_new) {
			boss_phase = phase_new;
			boss_phase_frame = 0;
			hit.invincibility_frame = 0;
			pattern_cur = 0;
		}
	} phase;
	static bool initial_hp_rendered;

	Missiles.unput_update_render();
	Particles.unput_update_render(PO_TOP_RIGHT, V_WHITE);

	// Entrance animation
	if(boss_phase == 0) {
#ifdef TH01_NATIVE_ORB_PHYSICS
        hit={};phase={};
#endif
		page_access(0);
		boss_phase_frame = 0;
		ent_still.hitbox_orb_inactive = false;
		bool16 done = false; // (redundant)
		spreadin_interval = 1;
		spreadin_speed = 2;
		meteor_active = true; // (redundant)
		while(1) {
			boss_phase_frame++;
			if(!done) {
				phase_spreadin(BASE_LEFT, BASE_TOP);
			}
			if(boss_phase_frame == 0) {
				done++;
			}
			if(done == true) {
				break;
			}
			frame_delay(2);
		}
		boss_phase = 1;
		phase.pattern_cur = 0;
		initial_hp_rendered = false;

		// Doing some static initialization that wouldn't have been necessary
		// if this function was coded properly...
		pattern_hop_and_fire_chase_pellets(false);
	} else if(boss_phase == 1) {
		// ZUN bug: The fight only ends in Phase 3, which can in turn only ever
		// be entered after Mima completed her current pattern. Thus, HP
		// subtraction in debug mode can lead to this function being called
		// with [boss_phase_frame] being larger than the initial HP value,
		// causing the heap corruption bug mentioned in the comment of this
		// function. Mima starts with an even number of total HP, so this will
		// even happen for the easiest possible case of holding ↵ Return for
		// the first 6 frames of phase 1.
		hud_hp_increment_render(initial_hp_rendered, boss_hp, boss_phase_frame);

		phase.frame_common();
		meteor_put();
		if(phase.pattern_cur == 0) {
			pattern_aimed_then_static_pellets_from_square_corners();
		} else if(phase.pattern_cur == 1) {
			pattern_aimed_missiles_from_square_corners();
		} else if(phase.pattern_cur == 2) {
			pattern_static_pellets_from_corners_of_two_squares();
		} else if(phase.pattern_cur == 3) {
			pattern_hop_and_fire_chase_pellets();
		}

		// There might either be white squares on screen, or the hop pattern
		// might have manipulated the hardware palette. Delay the end of the
		// phase until the current pattern is done to make sure that everything
		// is cleaned up.
		if(boss_phase_frame == 0) {
			phase.pattern_next(4);
			if(boss_hp <= HP_PHASE_1_END) {
				phase.pattern_cur = 99;
			}
		}

		hit.update_and_render(flash_colors);
		if((phase.pattern_cur == 99) && !hit.invincible) {
			mima_unput_both();
			spreadin_interval = 4;
			spreadin_speed = 8;

			// ZUN bloat: Already done by phase_spreadin() in phase 2.
			ent_still.pos_cur_set(BASE_LEFT, BASE_TOP);
			mima_bg_snap();

			phase.next(2);
		}
	} else if(boss_phase == 2) {
		phase.frame_common();
		phase_spreadin(BASE_LEFT, BASE_TOP);
		if(boss_phase_frame == 0) {
			phase.next(3);
		}
	} else if(boss_phase == 3) {
		phase.frame_common();
		meteor_put();
		if(phase.pattern_cur == 0) {
			pattern_pillars_and_aimed_spreads();
		} else if(phase.pattern_cur == 1) {
			pattern_halfcircle_missiles_downwards_from_corners();
		} else if(phase.pattern_cur == 2) {
			pattern_slow_pellet_spray_from_corners();
		} else if(phase.pattern_cur == 3) {
			pattern_aimed_lasers_from_corners();
		}

		if(boss_phase_frame == 0) {
			phase.pattern_next(4);
		}

		hit.update_and_render(flash_colors);
		if(boss_hp <= HP_PHASE_3_END) {
			page_access(1);
			mima_unput();
			page_access(0);

			mdrv2_bgm_fade_out_nonblock();
			Pellets.unput_and_reset_nonclouds();
			Missiles.reset();
			shootout_lasers_unput_and_reset_broken(i, 5); // 5? Doubly broken...
			boss_defeat_animate();
		}
	}
#ifdef TH01_NATIVE_ORB_PHYSICS
    native_mima_observe(phase.pattern_cur,hit.invincibility_frame);
#endif
}

#ifdef TH01_NATIVE_ORB_PHYSICS
} // namespace mima_original
#endif
