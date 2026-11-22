/// Makai Stage 20 Boss - Sariel
/// ----------------------------

#ifdef TH01_NATIVE_ORB_PHYSICS
#include "native/sariel_backend.hpp"
namespace sariel_original {
#else
typedef int sariel_frame_t;
#include <stdlib.h>
#include <dos.h>
#include "libs/master.lib/pc98_gfx.hpp"
#include "th01/rank.h"
#include "th01/resident.hpp"
#include "th01/v_colors.hpp"
#include "th01/math/dir.hpp"
#include "th01/math/overlap.hpp"
#include "th01/math/vector.hpp"
#include "th01/hardware/frmdelay.h"
#include "th01/hardware/graph.h"
#include "th01/hardware/grcg8x8m.hpp"
#include "th01/hardware/grpinv32.hpp"
#include "th01/hardware/palette.h"
#include "th01/hardware/pgtrans.hpp"
#include "th01/hardware/scrollup.hpp"
#include "th01/snd/mdrv2.h"
#include "th01/formats/grc.hpp"
#include "th01/formats/grp.h"
#include "th01/sprites/leaf.hpp"
#include "th01/sprites/pellet.h"
#include "th01/sprites/shape8x8.hpp"
#include "th01/main/shape.hpp"
#include "th01/main/spawnray.hpp"
#include "th01/main/player/player.hpp"
#include "th01/main/stage/palette.hpp"
#include "th01/main/stage/stageobj.hpp"
#include "th01/main/stage/stages.hpp"
#include "th01/main/player/bomb.hpp"
#include "th01/main/player/shot.hpp"
#include "th01/main/boss/entity_a.hpp"
#include "th01/main/boss/palette.hpp"
#include "th01/main/bullet/laser_s.hpp"
#include "th01/main/bullet/pellet.hpp"
#include "th01/main/hud/hp.hpp"
#include "th01/main/hud/hud.hpp"
#include "platform/x86real/pc98/egc.hpp"
#include "platform/x86real/pc98/page.hpp"

#endif

static const char* unused_entrance_letters_maybe[] = { "ANGEL", "OF", "DEATH" };

// Coordinates
// -----------

#include "th01/main/boss/sariel_coords.inc"
enum sariel_colors_t {
	COL_LASER = 4,
	COL_PARTICLE2X2 = 4,
	COL_DETONATION = 5,
	COL_DETONATION_START = 6,
	COL_FORM2_PULSE = 6,
	COL_AIR = 12,
	COL_BIRD = 15, // Yes, just a single one, changed by the background image.
};

static union {
	sariel_frame_t frame;
	sariel_frame_t speed_multiplied_by_8;
	sariel_frame_t interval;
	sariel_frame_t start_frame;
	pixel_t velocity_x;
	sariel_frame_t pellet_count;
	sariel_frame_t unknown;
} pattern_state;

// File names
// ----------

#ifndef TH01_NATIVE_ORB_PHYSICS
#include "th01/shiftjis/fns.hpp"
#endif

const char* BG_IMAGES[4] = {
	"BOSS6_A1.GRP", "BOSS6_A2.GRP", "BOSS6_A3.GRP", "BOSS6_A4.GRP"
};
// ----------

// Entities (and animations)
// --------

#include "th01/main/boss/sariel_entities.inc"
/// .GRC entities
/// -------------

static const main_grc_slot_t GRC_SLOT_VORTEX_DEBRIS = GRC_SLOT_BOSS_1;
static const main_grc_slot_t GRC_SLOT_BIRD = GRC_SLOT_BOSS_2;
static const main_grc_slot_t GRC_SLOT_SPAWNCROSS = GRC_SLOT_BOSS_3;
static const main_grc_slot_t GRC_SLOT_LEAFSPLASH = GRC_SLOT_BOSS_4;

inline void sariel_grc_load(void) {
	grc_load(GRC_SLOT_VORTEX_DEBRIS, "boss6gr1.grc");
	grc_load(GRC_SLOT_BIRD, "boss6gr2.grc");
	grc_load(GRC_SLOT_SPAWNCROSS, "boss6gr3.grc");
	grc_load(GRC_SLOT_LEAFSPLASH, "boss6gr4.grc");
}

inline void sariel_grc_free(void) {
	grc_free(GRC_SLOT_VORTEX_DEBRIS);
	grc_free(GRC_SLOT_BIRD);
	grc_free(GRC_SLOT_SPAWNCROSS);
	grc_free(GRC_SLOT_LEAFSPLASH);
}

// Vortex and debris sprites (BOSS6GR1.GRC)
// ----------------------------------------

#include "th01/main/boss/sariel_vortex_sprites.inc"

// Birds (BOSS6GR2.GRC)
// --------------------

static const pixel_t BIRD_W = 32;
static const pixel_t BIRD_H = 32;

static const int BIRD_COUNT = 30;
static const int BIRD_HATCH_CELS = 5;
static const int BIRD_FLY_CELS = 2;

enum bird_cel_t {
	C_EGG = 0,
	C_HATCH,
	C_HATCH_last = (C_HATCH + BIRD_HATCH_CELS - 1),
	C_FLY,
	C_FLY_last = (C_FLY + BIRD_FLY_CELS - 1),
};

#define bird_put_8(left, top, cel) \
	grc_put_8(left, top, GRC_SLOT_BIRD, cel, COL_BIRD);
// --------------------

// Spawn cross (BOSS6GR3.GRC)
// --------------------------

static const pixel_t SPAWNCROSS_W = 32;
static const pixel_t SPAWNCROSS_H = 32;
static const int SPAWNCROSS_CELS = 2;
// --------------------------

// Leaf splash animation (BOSS6GR4.GRC)
// ------------------------------------
// Never seen in the original game, due to a ZUN bug in the only pattern that
// uses it.

static const pixel_t LEAFSPLASH_W = 32;
static const pixel_t LEAFSPLASH_H = 32;
static const int LEAFSPLASH_CELS = 3;
// ------------------------------------
/// -------------

// .PTN
// ----

// The original wand sprite together with its background, as it appears in the
// .GRP images, snapped from VRAM.
static const main_ptn_slot_t PTN_SLOT_WAND_LOWERED = PTN_SLOT_BOSS_1;
// ----

// 2×2 particles (just "particles" was taken)
// -------------

#include "th01/main/boss/sariel_particle_macros.inc"

#ifndef TH01_NATIVE_ORB_PHYSICS
#include "th01/main/select_r.cpp"
#endif

#include "th01/main/boss/sariel_entrance.inc"
void sariel_load_and_init(void)
{
	sariel_ent_load();
	sariel_grc_load();
	boss_palette_snap();
	void sariel_setup(void);
	sariel_setup();
	ptn_new(
		PTN_SLOT_WAND_LOWERED, (((WAND_W / PTN_W) * (WAND_H / PTN_H)) + 4)
	); // ?
}

#include "th01/main/boss/sariel_setup.inc"
#include "th01/main/boss/sariel_wand_background.inc"
void sariel_free(void)
{
	sariel_ent_free();
	sariel_grc_free();
	ptn_free(PTN_SLOT_WAND_LOWERED);
}

// Almost identical to Konngara's version. This one is worse.
#include "th01/main/boss/sariel_spawnrays.inc"

#include "th01/main/boss/sariel_birds.inc"

#include "th01/main/boss/sariel_shield.inc"
// Renders a frame of Sariel's wand raise animation on both VRAM pages, and
// returns `true` once the animation completed.
#include "th01/main/boss/sariel_wand_raise.inc"
// Should maybe return `false`, for consistency with wand_render_raise_both().
#include "th01/main/boss/sariel_wand_lower.inc"
#include "th01/main/boss/sariel_dress.inc"
#include "th01/main/boss/sariel_vortices.inc"

#include "th01/main/boss/sariel_random_lasers.inc"

#include "th01/main/boss/sariel_bird_arc.inc"

#include "th01/main/boss/sariel_background.inc"

#include "th01/main/boss/sariel_particles_vertical.inc"

#include "th01/main/boss/sariel_snowflake.inc"

#include "th01/main/boss/sariel_orb_rings.inc"

#include "th01/main/boss/sariel_sling_clusters.inc"

#include "th01/main/boss/sariel_particles_wavy.inc"

#include "th01/main/boss/sariel_four_lasers.inc"

void near shake_for_50_frames(void)
{
	if(boss_phase_frame < 50) {
		if((boss_phase_frame % 8) == 0) {
			z_vsync_wait_and_scrollup(RES_Y - 4);
		} else if((boss_phase_frame % 8) == 4) {
			z_vsync_wait_and_scrollup(RES_Y + 4);
		}
		if((boss_phase_frame % 4) == 0) {
			mdrv2_se_play(7);
		}
	} else if(boss_phase_frame == 50) {
		z_vsync_wait_and_scrollup(0);
		boss_phase_frame = 0;
	}
}

#include "th01/main/boss/sariel_top_rain.inc"

#include "th01/main/boss/sariel_radial_stacks.inc"

#include "th01/main/boss/sariel_bottom_birds.inc"

#include "th01/main/boss/sariel_semicircles.inc"

#include "th01/main/boss/sariel_columns.inc"

#include "th01/main/boss/sariel_dotted_circle.inc"

#include "th01/main/boss/sariel_particles_horizontal.inc"

#include "th01/main/boss/sariel_curved_spray.inc"

#include "th01/main/boss/sariel_seal_rain.inc"

#include "th01/main/boss/sariel_double_spray.inc"

#include "th01/main/boss/sariel_leaves.inc"

void sariel_main(void)
{
#ifdef TH01_NATIVE_ORB_PHYSICS
	if(game_cleared) return;
#endif
	enum {
		PHASE_FORM1_DEFEATED = 99,
		PHASE_FORM2 = 100,
		PHASE_FORM2_DEFEATED = 101,

		MAGNITUDE = 16,
	};

	static bool16 invincible;
	static sariel_frame_t invincibility_frame;
	static pixel_t entrance_ring_radius_base;
	static bool initial_hp_rendered;
	static struct {
		int pattern_cur;
		union {
			int patterns_done;
			int pulse_fade_direction;
		} u1;
		int patterns_until_next;

		void next(int phase) {
			boss_phase = phase;
			boss_phase_frame = 0;
			// Shouldn't this be bg_transition()'s job?
			wand_lowered_snap();
		}

		void frame_common(bool form2) {
			boss_phase_frame++;
			invincibility_frame++;
			if(!form2) {
				shield_render_both();
				dress_render_both();
			}
		}

		void frame_bg_transition(int next_bg_id) {
			boss_phase_frame++;
			bg_transition(next_bg_id);
			dress_render_both();
		}

		void pattern_next(int ring_min, int ring_max) {
			if(ring_max == 1) {
				pattern_cur = (1 - pattern_cur);
			} else {
				pattern_cur = (pattern_cur == ring_max)
					? ring_min
					: (pattern_cur + 1);
			}
			// ZUN quirk: Modifying this variable during the second form also
			// causes the pulse effect to switch its fade direction. Might have
			// even been sort of intended.
			u1.patterns_done++;
		}
	} phase = { 0, 0, 0 };

	#define phase_form1_next_if_done(next_phase) { \
		if(boss_hp <= 0) { \
			boss_phase = PHASE_FORM1_DEFEATED; \
		} \
		if( \
			(phase.u1.patterns_done >= phase.patterns_until_next) && \
			!invincible \
		) { \
			phase.u1.patterns_done = 0; \
			boss_phase = next_phase; \
			phase.pattern_cur = 0; \
			boss_phase_frame = 0; \
		} \
	}

#ifdef TH01_NATIVE_ORB_PHYSICS
	struct NativeObservation { decltype(phase)& state; bool16& inv; sariel_frame_t& frame; ~NativeObservation() { native_sariel_observe(state.pattern_cur, state.u1.patterns_done, frame, inv != 0); } } observation{phase, invincible, invincibility_frame};
#endif

	unsigned int i;
	const vc_t flash_colors[3] = { 3, 4, 5 };

	struct {
		bool padding;
		bool colliding_with_orb;

		void update_and_render(const vc_t (&flash_colors)[3]) {
			boss_hit_update_and_render(
				invincibility_frame,
				invincible,
				boss_hp,
				flash_colors,
				(sizeof(flash_colors) / sizeof(flash_colors[0])),
				10000,
				colliding_with_orb,
				shot_hitbox_t(
					FACE_LEFT, FACE_TOP, ((FACE_W * 5) / 4), ((FACE_H * 3) / 4)
				)
			);
		}
	} hit;

	hit.colliding_with_orb = overlap_xy_xywh_le_ge(
		orb_cur_left, orb_cur_top,
		FACE_LEFT, FACE_TOP, (FACE_W - ORB_W), (FACE_H - ORB_H)
	) || ent_shield.hittest_orb();

	if(boss_phase == 0) {
		boss_phase_frame = -1;
		invincibility_frame = 0;
		invincible = false;
		boss_palette_snap();
		stage_palette_set(z_Palettes);
		random_seed = frame_rand;

		while(1) {
			#define frame_half	boss_phase_frame

			unsigned int tmp;

			frame_half++;
			if(entrance_rings_update_and_render(
				entrance_ring_radius_base, i, tmp, frame_half, 16, 1
			)) {
				boss_phase = 1;
				phase.pattern_cur = 0;
				phase.u1.patterns_done = 0;
				phase.patterns_until_next = ((irand() % 6) + 1);
				boss_phase_frame = 0;
				initial_hp_rendered = 0;
				boss_palette_show(); // Unnecessary.
				ent_shield.pos_cur_set(SHIELD_LEFT, SHIELD_TOP);
				wand_lowered_snap();
				wand_render_raise_both(true);
				Birds.reset();
				break;
			}
entrance_rings_still_active:
			if(frame_half % 2) { // That's why we've renamed the variable
				frame_delay(1);
			}
			#undef frame_half
		}
	} else if(boss_phase == 1) {
		hud_hp_increment_render(initial_hp_rendered, boss_hp, boss_phase_frame);
		phase.frame_common(false);
		Birds.unput_update_render();

		if(phase.pattern_cur == 0) {
			pattern_random_purple_lasers();
		} else if(phase.pattern_cur == 1) {
			pattern_vortices();
		} else if(phase.pattern_cur == 2) {
			pattern_birds_on_ellipse_arc();
		}

		if(boss_phase_frame == 0) {
			phase.pattern_next(0, 2);
		}
		hit.update_and_render(flash_colors);
		phase_form1_next_if_done(2);
	} else if(boss_phase == 2) {
		phase.frame_bg_transition(1);
		if(boss_phase_frame == 0) {
			phase.next(3);
			// Assume that the palette didn't change between background ID 0
			// and 1...
			// boss_palette_snap();
			phase.patterns_until_next = ((irand() % 5) + 1);
		}
	} else if(boss_phase == 3) {
		phase.frame_common(false);
		particles2x2_vertical_unput_update_render(false);

		if(phase.pattern_cur == 0) {
			pattern_detonating_snowflake();
			pattern_2_rings_from_a2_orbs();
		} else if(phase.pattern_cur == 1) {
			pattern_aimed_sling_clusters();
			pattern_2_rings_from_a2_orbs();
		} else if(boss_phase_frame < 60) {
			boss_phase_frame = 0;
		}

		if(boss_phase_frame == 0) {
			phase.pattern_next(0, 1);
		}
		hit.update_and_render(flash_colors);
		phase_form1_next_if_done(4);
	} else if(boss_phase == 4) {
		phase.frame_bg_transition(2);
		if(boss_phase_frame == 0) {
			phase.next(5);
			boss_palette_snap();
			phase.patterns_until_next = ((irand() % 4) + 3);
		}
	} else if(boss_phase == 5) {
		phase.frame_common(false);
		particles2x2_wavy_unput_update_render();

		if(phase.pattern_cur == 0) {
			pattern_four_aimed_lasers();
		} else if(phase.pattern_cur == 1) {
			shake_for_50_frames();
		} else if(phase.pattern_cur == 2) {
			pattern_four_aimed_lasers();
			pattern_rain_from_top();
		} else if(phase.pattern_cur == 3) {
			pattern_radial_stacks_and_lasers();
		} else if(phase.pattern_cur == 4) {
			shake_for_50_frames();
		} else if(phase.pattern_cur == 5) {
			pattern_four_aimed_lasers();
			pattern_rain_from_top();
		}

		if(boss_phase_frame == 0) {
			phase.pattern_next(10, 5); // ???
		}
		hit.update_and_render(flash_colors);
		phase_form1_next_if_done(6);
	} else if(boss_phase == 6) {
		phase.frame_bg_transition(3);
		if(boss_phase_frame == 0) {
			phase.next(7);
			boss_palette_snap();
			phase.patterns_until_next = ((irand() % 5) + 2);
		}
	} else if(boss_phase == 7) {
		phase.frame_common(false);
		particles2x2_vertical_unput_update_render(true);
		Birds.unput_update_render();

		if(phase.pattern_cur == 0) {
			pattern_symmetric_birds_from_bottom();
		} else if(phase.pattern_cur == 1) {
			pattern_four_semicircle_spreads();
		} else if(phase.pattern_cur == 2) {
			pattern_vertical_stacks_from_bottom_then_random_rain_from_top();
		}

		if(boss_phase_frame == 0) {
			phase.pattern_next(0, 2);
		}
		hit.update_and_render(flash_colors);
		phase_form1_next_if_done(8);
	} else if(boss_phase == 8) {
		phase.frame_bg_transition(0);
		if(boss_phase_frame == 0) {
			phase.next(1);
			boss_palette_snap();
			phase.patterns_until_next = ((irand() % 6) + 1);
		}
	} else if(boss_phase == PHASE_FORM1_DEFEATED) {
		boss_phase_frame = 0;

		// Not that this variable is ever read from, before it's set back to 0
		// at the end of the transition animation.
		invincibility_frame = 399;

		Shots.unput_and_reset();
		Pellets.unput_and_reset_nonclouds();
		shootout_lasers_unput_and_reset_broken(i, SHOOTOUT_LASER_COUNT);

		// MODDERS: Move this to a common player reset function.
		orb_cur_left = ORB_LEFT_START;
		orb_cur_top = ORB_TOP_START;
		player_left = PLAYER_LEFT_START;
		orb_force = ORB_FORCE_START;
		orb_force_frame = 0;
		orb_velocity_x = ORB_VELOCITY_X_START;
		player_deflecting = false;
		bomb_damaging = false;
		player_reset();

		invincible = false;
		random_seed = frame_rand;
		mdrv2_bgm_fade_out_nonblock();

		// boss6.grp is not part of the game? Might have been a defeat graphic.
		page_access(1);
		grp_put("boss6.grp", GPF_PALETTE_SHOW);
		z_palette_set_show(0xF, 0x0, 0x0, 0x0);
		boss_palette_snap();
		graph_copy_page_to_other(1);

		page_access(1);
		grp_put("boss6_a5.grp", GPF_PALETTE_KEEP);
		page_access(0);

		while(1) {
			boss_phase_frame++;
			if(boss_phase_frame < 200) {
				if((boss_phase_frame % 2) == 0) {
					z_palette_set_show(
						0xF, RGB4::max(), RGB4::max(), RGB4::max()
					);
					z_vsync_wait_and_scrollup(RES_Y + MAGNITUDE);
				}
				if((boss_phase_frame % 2) == 1) {
					boss_palette_show();
					z_vsync_wait_and_scrollup(RES_Y - (MAGNITUDE - 2)); // ???
				}
				if((boss_phase_frame % 4) == 0) {
					mdrv2_se_play(9);
				}
			}
			if(boss_phase_frame == 200) {
				boss_palette_show();
				z_vsync_wait_and_scrollup(0);
			}
			if(boss_phase_frame > 200) {
				for(i = 0; i < COMPONENT_COUNT; i++) {
					grp_palette[0xF].v[i] = 0x0;
				}
				// The palette was loaded from boss6_a5.grp earlier.
				pagetrans_diagonal_8x8_with_palette(0, z_Palettes, grp_palette);
				page_access(1);
				mdrv2_bgm_load("syugen.MDT");
				mdrv2_bgm_play();
				grp_put("boss6_a6.grp", GPF_PALETTE_SHOW);
				z_palette_set_show(COL_FORM2_PULSE, 0x0, 0x0, 0x0);
				graph_copy_page_to_other(1);
				page_access(1);
				hud_rerender();
				z_vsync_wait_and_scrollup(0);
				boss_phase_frame = 0;
				invincibility_frame = 0;
				phase.pattern_cur = 0;
				phase.u1.pulse_fade_direction = 0;
				boss_phase = PHASE_FORM2;
				player_invincibility_time = 0;
				player_invincible = false;
				boss_palette_snap();
				stage_palette_set(z_Palettes);

				boss_hp = 6;
				hud_hp_first_white = 10;
				hud_hp_first_redwhite = 3;
				initial_hp_rendered = false;
				pattern_swaying_leaves(boss_phase_frame, 999);
				return;
			} else {
				frame_delay(1);
			}
		}
	} else if(boss_phase == PHASE_FORM2) {
		hud_hp_increment_render(initial_hp_rendered, boss_hp, boss_phase_frame);
		phase.frame_common(true);
		particles2x2_horizontal_unput_update_render(boss_phase_frame);

		if(phase.pattern_cur == 0) {
			// Hey, that variable is supposed to be off-limits to the main
			// function! (And also completely pointless, since it could have
			// just been done locally for pattern 4.)
			select_for_rank(pattern_state.interval, 56, 32, 24, 20);

			if(boss_phase_frame > 100) {
				boss_phase_frame = 0;
			}
		} else if(phase.pattern_cur == 1) {
			pattern_curved_spray_leftright_once(boss_phase_frame);
		} else if(phase.pattern_cur == 2) {
			pattern_rain_from_seal_center(boss_phase_frame);
		} else if(phase.pattern_cur == 3) {
			pattern_curved_spray_leftright_twice(boss_phase_frame);
		} else if(phase.pattern_cur == 4) {
			pattern_swaying_leaves(boss_phase_frame, pattern_state.interval);
			if(boss_phase_frame > 300) {
				boss_phase_frame = 0;
			}
		} else if(phase.pattern_cur == 5) {
			pattern_swaying_leaves(boss_phase_frame, 500);
			if(boss_phase_frame > 200) {
				boss_phase_frame = 0;
			}
		} else if(phase.pattern_cur == 6) {
			pattern_swaying_leaves(boss_phase_frame, 500);
			pattern_curved_spray_leftright_once(boss_phase_frame);
		} else if(phase.pattern_cur == 7) {
			pattern_swaying_leaves(boss_phase_frame, 500);
			pattern_rain_from_seal_center(boss_phase_frame);
		} else if(phase.pattern_cur == 8) {
			pattern_swaying_leaves(boss_phase_frame, 32);
			pattern_curved_spray_leftright_twice(boss_phase_frame);
		}

		if(boss_phase_frame == 0) {
			phase.pattern_next(4, 8);
		}
		hit.update_and_render(flash_colors);

		if((boss_phase_frame % 10) == 0) {
			// Setting just the COL_FORM2_PULSE entry in this palette surely
			// doesn't require resetting the hardware palette to the intended
			// boss palette, modifying the one entry, and then capturing the
			// palette again...
			boss_palette_show();
			if(phase.u1.pulse_fade_direction == 0) {
				if(z_Palettes[COL_FORM2_PULSE].c.r < 0xA) {
					z_Palettes[COL_FORM2_PULSE].c.r++;
				} else {
					phase.u1.pulse_fade_direction = 1;
				}
				if(z_Palettes[COL_FORM2_PULSE].c.g < 0xA) {
					z_Palettes[COL_FORM2_PULSE].c.g++;
				}
				if(z_Palettes[COL_FORM2_PULSE].c.b < 0x6) {
					z_Palettes[COL_FORM2_PULSE].c.b++;
				}
			} else {
				if(z_Palettes[COL_FORM2_PULSE].c.r > 0x0) {
					z_Palettes[COL_FORM2_PULSE].c.r--;
				} else {
					phase.u1.pulse_fade_direction = 0;
				}
				if(z_Palettes[COL_FORM2_PULSE].c.g > 0x0) {
					z_Palettes[COL_FORM2_PULSE].c.g--;
				}
				if(z_Palettes[COL_FORM2_PULSE].c.b > 0x0) {
					z_Palettes[COL_FORM2_PULSE].c.b--;
				}
			}
			// Yeah, what a waste.
			z_palette_set_all_show(z_Palettes);
			boss_palette_snap();
		}

		if(boss_hp <= 0) {
			mdrv2_bgm_stop();
			boss_phase = PHASE_FORM2_DEFEATED;
			boss_phase_frame = 0;
			invincibility_frame = 0;
		}
	} else if(boss_phase == PHASE_FORM2_DEFEATED) {
		page_access(1);
		grp_put("boss6_a6.grp", GPF_PALETTE_SHOW);

		// Actually a different color inside the .GRP! Would have been nicer to
		// reuse the previous state of the color from above.
		z_palette_set_show(COL_FORM2_PULSE, 0x0, 0x0, 0x0);

		boss_palette_snap(); // No longer necessary.

		graph_copy_page_to_other(1);
		page_access(0);

		while(1) {
			boss_phase_frame++;
			if(boss_phase_frame < 230) {
				if((boss_phase_frame % 2) == 0) {
					z_vsync_wait_and_scrollup(RES_Y + MAGNITUDE);
				}
				if((boss_phase_frame % 2) == 1) {
					z_vsync_wait_and_scrollup(RES_Y - (MAGNITUDE - 2)); // ???
				}
				if((boss_phase_frame < 200) && ((boss_phase_frame % 4) == 0)) {
					mdrv2_se_play(9);
				}
			}
			if(boss_phase_frame == 170) {
				mdrv2_bgm_fade_out_nonblock();
			}
			if(boss_phase_frame > 190) {
				if((boss_phase_frame % 2) == 0) {
					unsigned int comp;
					z_palette_black_out_step_bugged(i, comp);
				}
				if(boss_phase_frame > 230) {
					break;
				}
			}
			frame_delay(2);
		}

		z_vsync_wait_and_scrollup(0);
		sariel_free();

		// Makai clear! Grant 50,000 score points in the fanciest way
		for(i = 0; i < 5; i++) {
			score += 10000;
		}
		game_cleared = true;
	}

	#undef phase_form1_next_if_done
}

#ifdef TH01_NATIVE_ORB_PHYSICS
void native_sariel_reset() {
    native_particle_reset = true;
    particles2x2_vertical_unput_update_render(false);
    particles2x2_wavy_unput_update_render();
    particles2x2_horizontal_unput_update_render(0);
    native_particle_reset = false;
    native_actor_reset = true;
    pattern_vortices();
    pattern_birds_on_ellipse_arc();
    native_actor_reset = false;
    sariel_frame_t frame = 0;
    pattern_swaying_leaves(frame, 999);
    wand_render_raise_both(true);
    Birds = {};
    game_cleared = false;
}
#endif

#ifdef TH01_NATIVE_ORB_PHYSICS
} // namespace sariel_original
#endif
