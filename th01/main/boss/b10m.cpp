/// Makai Stage 10 Boss - YuugenMagan
/// ---------------------------------

#ifdef TH01_NATIVE_ORB_PHYSICS
#include "native/yuugenmagan_backend.hpp"
namespace yuugenmagan_original {
#else
#include "th01/main/boss/yuugenmagan_types.hpp"
#include "decomp.hpp"
#include "th01/rank.h"
#include "th01/resident.hpp"
#include "th01/v_colors.hpp"
#include "th01/math/dir.hpp"
#include "th01/math/overlap.hpp"
#include "th01/math/polar.hpp"
#include "th01/math/vector.hpp"
#include "th01/hardware/graph.h"
#include "th01/hardware/palette.h"
#include "th01/snd/mdrv2.h"
#include "th01/sprites/pellet.h"
#include "th01/main/boss/defeat.hpp"
#include "th01/main/boss/entity_a.hpp"
#include "th01/main/boss/palette.hpp"
#include "th01/main/bullet/line.hpp"
#include "th01/main/bullet/missile.hpp"
#include "th01/main/bullet/pellet.hpp"
#include "th01/main/hud/hp.hpp"
#include "th01/main/player/player.hpp"
#include "th01/main/stage/palette.hpp"

#endif
// Coordinates
// -----------

#include "th01/main/boss/yuugenmagan_eye.inc"

#ifdef TH01_NATIVE_ORB_PHYSICS
static CEyeEntity native_eyes[5]{};
#define eye_west native_eyes[0]
#define eye_east native_eyes[1]
#define eye_southwest native_eyes[2]
#define eye_southeast native_eyes[3]
#define eye_north native_eyes[4]
#else
#define eye_west     	reinterpret_cast<CEyeEntity &>(boss_entity_0)
#define eye_east     	reinterpret_cast<CEyeEntity &>(boss_entity_1)
#define eye_southwest	reinterpret_cast<CEyeEntity &>(boss_entity_2)
#define eye_southeast	reinterpret_cast<CEyeEntity &>(boss_entity_3)
#define eye_north    	reinterpret_cast<CEyeEntity &>(boss_entity_4)

#endif
#include "th01/main/boss/yuugenmagan_eye_helpers.inc"

// .PTN
// ----

static const main_ptn_slot_t PTN_SLOT_MISSILE = PTN_SLOT_BOSS_1;
// ----

// Patterns
// --------

// Yes! Finally a proper stateless version of this concept!
#include "th01/main/boss/yuugenmagan_rank.inc"

void yuugenmagan_load(void)
{
#ifdef TH01_NATIVE_ORB_PHYSICS
    native_yuugenmagan_reset();
    for(auto& eye:native_eyes)eye={};
#endif
	yuugenmagan_ent_load();
	void yuugenmagan_setup(void);
	yuugenmagan_setup();
	Missiles.load(PTN_SLOT_MISSILE);
	Missiles.reset();
}

#include "th01/main/boss/yuugenmagan_setup.inc"

void yuugenmagan_free(void)
{
	yuugenmagan_ent_free();
	ptn_free(PTN_SLOT_MISSILE);
}

// Phases
// ------

#include "th01/main/boss/yuugenmagan_intro_helpers.inc"

#include "th01/main/boss/yuugenmagan_lateral.inc"

// Aimed missiles
// --------------

#include "th01/main/boss/yuugenmagan_missiles.inc"

// Lasers across the playfield
// ---------------------------

#include "th01/main/boss/yuugenmagan_lasers.inc"

/// Pentagram
/// ---------

#include "th01/main/boss/yuugenmagan_pentagram.inc"

// Function ordering fails
// -----------------------

// Opens and closes the given eyes, and changes the 邪 color in [stage_palette]
// (and, via transferring it to [z_Palettes], also the hardware color) by
// gradually incrementing and decrementing the given components on 9 out of 10
// frames. Takes ownership of [frame]. To keep the 邪 color unchanged, set
// [yokoshima_comp_inc] and [yokoshima_comp_dec] to a value ≥ COMPONENT_COUNT.
//
// ZUN quirk: While the final [z_Palettes] values are clamped to 0x0 and 0xF,
// the [yokoshima_comp_inc] component in the [stage_palette] is *not* clamped
// to 0xF after incrementing. This overflow can (and does) affect the final
// color on successive recoloring operations.
//
// MODDERS: Make the component parameters unsigned.
void eyes_toggle_and_yokoshima_recolor(
	eye_flag_t eyes_to_close,
	eye_flag_t eyes_to_open,
	int yokoshima_comp_dec,
	int yokoshima_comp_inc,
	yuugenmagan_int_t& frame
);
// -----------------------

// Separate function to work around the `Condition is always true/false` and
// `Unreachable code` warnings. (It's unnecessary anyway, though.)
inline void conditionally_reset_missiles(bool cond) {
	if(cond) {
		Missiles.reset();
	}
}

#define yuugenmagan_defeat_if(cond, reset_missiles, tmp_i) { \
	if(cond) { \
		mdrv2_bgm_fade_out_nonblock(); \
		Pellets.unput_and_reset_nonclouds(); \
		conditionally_reset_missiles(reset_missiles); \
		boss_defeat_animate(); \
	} \
}

void yuugenmagan_main(void)
{
	const vc_t flash_colors[2] = { 1, 11 };

	static yuugenmagan_int_t invincibility_frame;

	static union {
		yuugenmagan_int_t missile_pairs_fired_in_subphase;
		yuugenmagan_int_t subphase_frame;

		// Always increasing, even when going right-to-left.
		pixel_t x_edge_offset;

		pixel_t distance;
		yuugenmagan_int_t unused;
	} u1;

	static union {
		yuugenmagan_int_t subphase;
		yuugenmagan_int_t yokoshima_comp_dec;
		yuugenmagan_int_t unused;
	} u2;

	static screen_x_t target_left;
	static pixel_t unused_distance;

	// Compared to just reusing [invincibility_frame], this "copy" has the
	// advantage of not being reset every 40 frames, and thus lasting the full
	// EYE_TOGGLE_FRAMES.
	static yuugenmagan_int_t after_hit_frame;

	static Pentagram pentagram_;

	static union {
		int8_t iterations_done;
		int8_t yokoshima_comp_inc;
		eye_flag_t eyes_open;
		int8_t unused;
	} u3;

	static struct {
		bool initial_hp_rendered;

		void frame_common(void) {
			boss_phase_frame++;
			invincibility_frame++;
		}

		void next(yuugenmagan_int_t phase_new) {
			boss_phase = phase_new;
			boss_phase_frame = 0;
			invincibility_frame = 0;
		}

		void next(yuugenmagan_int_t phase_new, yuugenmagan_int_t& u2_element_to_reset) {
			boss_phase = phase_new;
			u2_element_to_reset = 0;
			boss_phase_frame = 0;
			invincibility_frame = 0;
		}

		void next(
			yuugenmagan_int_t phase_new,
			yuugenmagan_int_t& u2_element_to_reset,
			int8_t& u3_element_to_reset,
			int8_t new_value_for_u3 = 0
		) {
			boss_phase = phase_new;
			u2_element_to_reset = 0;
			u3_element_to_reset = new_value_for_u3;
			boss_phase_frame = 0;
			invincibility_frame = 0;
		}

		// Respawns the pentagram.
		void back_from_13_to_10(pentagram_attack_phase_t pentagram_phase_new) {
			if(pentagram_phase_new == PAP_PREPARE_1) {
				pentagram_.phase = pentagram_phase_new;
			}
			boss_phase_frame = 0;
			boss_phase = 10;
			u2.yokoshima_comp_dec = COMPONENT_COUNT;
			u3.yokoshima_comp_inc = COMPONENT_COUNT;
			if(pentagram_phase_new == PAP_PREPARE_2) {
				pentagram_.phase = pentagram_phase_new;
			}
			invincibility_frame = 0;
			u1.unused = 0;
			pentagram.unput();
		}
	} phase;

	static union {
		unsigned char missile_southwest;
		unsigned char pellet_east;
		unsigned char tmp; // MODDERS: Turn into a scope-local variable
	} angle;

	static unsigned char angle_missile_southeast;

	static struct {
		bool16 invincible;

		void update_and_render(const vc_t (&flash_colors)[2]) {
			#define hittest(eye) ( \
				(eye.hittest_orb() == true) && (eye.image() != C_HIDDEN) \
			)

			boss_hit_update_and_render(
				invincibility_frame,
				invincible,
				boss_hp,
				flash_colors,
				(sizeof(flash_colors) / sizeof(flash_colors[0])),
				5000,
				(
					hittest(eye_west) ||
					hittest(eye_east) ||
					hittest(eye_southwest) ||
					hittest(eye_southeast) ||
					hittest(eye_north)
				)
			);

			#undef hittest
		}
	} hit = { false };

#ifdef TH01_NATIVE_ORB_PHYSICS
    if(!native_yuugenmagan_before_frame())return;
    if(native_yuugenmagan_take_reset()){
        invincibility_frame=0;u1={};u2={};u3={};target_left=0;unused_distance=0;
        after_hit_frame=0;pentagram_={};phase={};hit={false};angle={};
        angle_missile_southeast=0;pentagram={};
    }
#endif
    Missiles.unput_update_render();

	if(boss_phase == 0) {
#include "th01/main/boss/yuugenmagan_intro_body.inc"
	} else if(boss_phase == 1) {
		// Slow pellets from the lateral eyes

		phase.frame_common();
		eyes_locked_unput_and_put_then_track(EF_WEST | EF_EAST);

		phase_1_pellets_from_lateral();
		hit.update_and_render(flash_colors);
		if((boss_hp <= HP_PHASE_1_END) || (boss_phase_frame > 1100)) {
			phase.next(2);
		}
	} else if(boss_phase == 2) {
		// (13, 13, 5) → (0, 13, 68) ⇒ #0DF (cyan)
		eyes_toggle_and_yokoshima_recolor(
			(EF_WEST | EF_EAST),
			(EF_SOUTHWEST | EF_SOUTHEAST),
			COMPONENT_R,
			COMPONENT_B,
			boss_phase_frame
		);
		if(boss_phase_frame >= EYE_TOGGLE_FRAMES) {
			phase.next(3, u2.subphase, u3.iterations_done);
			u1.missile_pairs_fired_in_subphase = 0;

			z_palette_set_all_show(stage_palette);
			boss_palette_snap();
			pattern_interval = select_for_rank(8, 12, 16, 20); // (unused)
		}
	} else if(boss_phase == 3) {
		// Missiles from the southern eyes, whose angles first shift away from
		// Reimu's tracked position and then towards it

		phase.frame_common();
		pattern_missile_pairs_from_south(
			u2.subphase,
			u1.missile_pairs_fired_in_subphase,
			target_left,
			angle.missile_southwest,
			angle_missile_southeast,
			u3.iterations_done,
			3,
			missile_pairs_shift_angle_1_away,
			missile_pairs_shift_angle_2_towards
		);
		hit.update_and_render(flash_colors);
		if((boss_hp <= HP_PHASE_3_END) || (u3.iterations_done >= 5)) {
			phase.next(4);
		}
	} else if(boss_phase == 4) {
		// (0, 13, 68) → (63, 0, 68) ⇒ #F0F (magenta)
		eyes_toggle_and_yokoshima_recolor(
			(EF_SOUTHWEST | EF_SOUTHEAST),
			(EF_WEST | EF_EAST),
			COMPONENT_G,
			COMPONENT_R,
			boss_phase_frame
		);
		if(boss_phase_frame >= EYE_TOGGLE_FRAMES) {
			phase.next(5, u2.subphase, u3.iterations_done);
			u1.subphase_frame = 0;

			z_palette_set_all_show(stage_palette);
			boss_palette_snap();
			unused_distance = (player_bottom() - eye_west.cur_top);
			pattern_interval = select_for_rank(12, 8, 4, 2);
			angle.pellet_east = 0x00;
		}
	} else if(boss_phase == 5) {
		// Circular pellets sprayed from the lateral eyes

		enum phase_5_subphase_t {
			P5_PREPARE_1 = 0,
			P5_SPRAY_1 = 1,
			P5_PREPARE_2 = 2,
			P5_SPRAY_2 = 3,
		};

		phase.frame_common();

		#include "th01/main/boss/yuugenmagan_spray.inc"
		hit.update_and_render(flash_colors);
		if((boss_hp <= HP_PHASE_5_END) || (u3.iterations_done > 4)) {
			phase.next(6);
		}
	} else if(boss_phase == 6) {
		// (63, 0, 68) → (0, 0, 68) ⇒ #00F (blue)
		eyes_toggle_and_yokoshima_recolor(
			(EF_WEST | EF_EAST),
			(EF_SOUTHWEST | EF_SOUTHEAST),
			COMPONENT_R,
			COMPONENT_COUNT,
			boss_phase_frame
		);
		if(boss_phase_frame >= EYE_TOGGLE_FRAMES) {
			phase.next(7, u2.subphase, u3.iterations_done);
			u1.missile_pairs_fired_in_subphase = 0;
			pentagram_.angle = 0x00; // ZUN bloat

			z_palette_set_all_show(stage_palette);
			boss_palette_snap();
			pattern_interval = select_for_rank(10, 16, 20, 24); // (unused)
		}
	} else if(boss_phase == 7) {
		// Missiles from the southern eyes, with (counter)clockwise angle shifts

		phase.frame_common();
		pattern_missile_pairs_from_south(
			u2.subphase,
			u1.missile_pairs_fired_in_subphase,
			target_left,
			angle.missile_southwest,
			angle_missile_southeast,
			u3.iterations_done,
			2,
			missile_pairs_shift_angle_1_clock,
			missile_pairs_shift_angle_2_clock
		);
		hit.update_and_render(flash_colors);
		if((boss_hp <= HP_PHASE_7_END) || (u3.iterations_done > 4)) {
			phase.next(8);
		}
	} else if(boss_phase == 8) {
		// (0, 0, 68) → (0, 63, 5) ⇒ #0F5 (green, via cyan)
		eyes_toggle_and_yokoshima_recolor(
			(EF_SOUTHWEST | EF_SOUTHEAST),
			EF_NORTH,
			COMPONENT_B,
			COMPONENT_G,
			boss_phase_frame
		);
		if(boss_phase_frame >= EYE_TOGGLE_FRAMES) {
			phase.next(9, u2.subphase);
			u1.x_edge_offset = 0;
			pentagram_.angle = 0x00;

			z_palette_set_all_show(stage_palette);
			boss_palette_snap();
		}
	} else if(boss_phase == 9) {
		// 3-pixel 3-laser sequence from the northern eye

		phase.frame_common();
		#include "th01/main/boss/yuugenmagan_laser_dispatch.inc"
		hit.update_and_render(flash_colors);
		if(
			(boss_hp <= HP_PHASE_9_END) ||
			(u2.subphase == laser_subphase(2, LSP_DONE))
		) {
			// ZUN bug: We can get here with a laser still on screen,
			// particularly when holding ↵ Return in debug mode. There should
			// at least be an unblitting call here.

			boss_phase = 10;
			boss_phase_frame = 0;
			invincibility_frame = 0;
			u2.yokoshima_comp_dec = COMPONENT_G;
			u3.yokoshima_comp_inc = COMPONENT_R;

			// Must be done here, as phase 13 loops back into phase 10 with
			// this phase variable set to PAP_PREPARE_2.
			pentagram_.phase = PAP_PREPARE_1;
		}
	} else if(boss_phase == 10) {
		// Prepare pentagram spawning by opening all eyes

		// First iteration: (0, 63, 5) → (63, 0, 5) ⇒ #F05 (red, via yellow)
		// Later iterations don't change the color; see back_from_13_to_10().
		eyes_toggle_and_yokoshima_recolor(
			EF_NONE,
			(EF_WEST | EF_EAST | EF_SOUTHWEST | EF_SOUTHEAST | EF_NORTH),
			u2.yokoshima_comp_dec,
			u3.yokoshima_comp_inc,
			boss_phase_frame
		);
		if(boss_phase_frame >= EYE_TOGGLE_FRAMES) {
			phase.next(11, u2.subphase, u3.unused, 31 /* ZUN bloat */);
			u1.distance = 0;
			pentagram_.angle = 0x00; // Finally not redundant.

			z_palette_set_all_show(stage_palette);
			boss_palette_snap();
		}
	} else if(boss_phase == 11) {
		// Spawns the pentagram with one corner out of every eye, then
		// gradually shrinks and moves it towards the center of the playfield

		enum phase_11_subphase_t {
			P11_PREPARE = 0,
			P11_CORNERS_IN_EYES = 1,
			P11_MOVE = 2,
			P11_DONE = 3,
		};

		phase.frame_common();
		#include "th01/main/boss/yuugenmagan_shrink.inc"
		hit.update_and_render(flash_colors);
		yuugenmagan_defeat_if((boss_hp <= HP_PHASE_13_END), false, i);
		if(
			((boss_hp <= HP_PHASE_13_END) || (u2.subphase == P11_DONE)) &&
			!hit.invincible
		) {
			phase.next(12, u2.unused, u3.eyes_open, EF_WEST);
			u1.unused = 0;
			pentagram.center.y = (pentagram.y[0] + PENTAGRAM_RADIUS_FINAL);
			pentagram.center.x = pentagram.x[0];
			pentagram.radius = PENTAGRAM_RADIUS_FINAL;

			// ZUN bug: This might look redundant, but it sets the coordinates
			// for the first unblitting call in phase 12. Which are wrong,
			// because PENTAGRAM_ANGLE_INITIAL is different.
			pentagram_corners_set_regular(0x00);

			// Work around the inaccuracies of 8-bit angles and make sure that
			// at least the lateral corners perfectly line up vertically.
			pentagram.y[1] = pentagram.y[4];

 			// Used in phase 13.
			pattern_interval = select_for_rank(24, 14, 10, 8);
		}
	} else if(boss_phase == 12) {
		// Moves the pentagram on top of the kimono figure

		enum {
			// Fixing the 8 pixels we've overshot above. In fact, the whole
			// reason for overshooting in the first place might have been to
			// ensure that *this* distance cleanly divides by 3?!
			DISTANCE_Y = ((
				PENTAGRAM_SHRINK_TARGET_CENTER_Y - PENTAGRAM_REGULAR_CENTER_Y
			) + 8),

			STEPS = (EYE_TOGGLE_FRAMES / PENTAGRAM_INTERVAL),
			Y_STEP = (DISTANCE_Y / STEPS),
		};

		// ZUN bloat: [boss_phase_frame] would have worked as well, and saved
		// the addition.
		invincibility_frame++;

		// ZUN quirk: Stays on red for the first iteration of the phase, due to
		// merely going
		//
		// 	(63, 0, 5) → (126, 0, 5) ⇒ #F05
		//
		// On the second iteration though, the same addition causes the red
		// component to overflow into negative numbers, which are clamped to
		// zero:
		//
		// 	(126, 0, 5) → (-67, 0, 5) ⇒ #005 (dark blue)
		//
		// The apparent pattern then repeats, leading to the color alternating
		// between red and dark blue on every 2.03 iterations if the player
		// manages to stall the fight long enough.
		eyes_toggle_and_yokoshima_recolor(
			(EF_EAST | EF_SOUTHWEST | EF_SOUTHEAST | EF_NORTH),
			EF_WEST,
			COMPONENT_G,
			COMPONENT_R,
			boss_phase_frame
		);
		if((invincibility_frame % PENTAGRAM_INTERVAL) == 0) {
			// ZUN bug: Missing an unblitting call for the final appearance of
			// the shrunk pentagram from the previous phase. This would
			// require:
			// • storing the shrink animation coordinates inside the structure
			//   rather than recalculating them for both unblitting and
			//   blitting operations,
			// • and not overwriting them with wrong ones at the end of the
			//   previous phase.

			pentagram.center.y -= Y_STEP;
			pentagram_regular_unput_update_render(PENTAGRAM_ANGLE_INITIAL);
		}
		if(boss_phase_frame >= EYE_TOGGLE_FRAMES) {
			phase.next(13, u2.unused, u3.eyes_open, EF_WEST);
			u1.unused = 0;

			// ZUN quirk: Should *maybe* have been PENTAGRAM_ANGLE_INITIAL to
			// achieve a seamless shift between moving up and spinning. Would
			// change the final pattern, though.
			pentagram_.angle = 0x00;

			after_hit_frame = 0;
			z_palette_set_all_show(stage_palette);
			boss_palette_snap();
		}
	} else if(boss_phase == 13) {
		enum {
			GROW_RADIUS = (SUBPHASE_TIMEOUT_FRAMES / PENTAGRAM_INTERVAL),
			GROWN_RADIUS = (PENTAGRAM_RADIUS_FINAL + GROW_RADIUS),
		};

		#define eye_is_open(bit) ( \
			u3.eyes_open & bit \
		)

		// ZUN bloat: Just a more pedantic version of the one above.
		#define eye_is_open_pedantic(bit) ( \
			eye_is_open(bit) == bit \
		)

		phase.frame_common();
		u1.unused++;
		eyes_foreach_if(eye_is_open_pedantic, locked_unput_and_put_8);
		if(hit.invincible && (after_hit_frame == 0)) {
			u3.eyes_open <<= 1;
			after_hit_frame = 1;
			if(u3.eyes_open == EF_RESET) {
				u3.eyes_open = EF_WEST;
			}
		} else if(after_hit_frame > 0) {
			eyes_toggle_and_yokoshima_recolor(
				(u3.eyes_open == EF_WEST) ? EF_NORTH : (u3.eyes_open >> 1),
				u3.eyes_open,
				COMPONENT_COUNT,
				COMPONENT_COUNT,
				after_hit_frame
			);
			if(after_hit_frame >= EYE_TOGGLE_FRAMES) {
				after_hit_frame = 0;
				u1.unused = 0;
			}
		} else {
			eyes_foreach_if(eye_is_open, track_player);
		}

		#include "th01/main/boss/yuugenmagan_regular.inc"
		hit.update_and_render(flash_colors);
		yuugenmagan_defeat_if((boss_hp <= HP_PHASE_13_END), true, i);

		#undef eye_is_open_pedantic
		#undef eye_is_open
	}
#ifdef TH01_NATIVE_ORB_PHYSICS
    native_yuugenmagan_observe(u2.subphase,u3.iterations_done,invincibility_frame,u3.eyes_open);
#endif
}

#include "th01/main/boss/yuugenmagan_transition.inc"

#ifdef TH01_NATIVE_ORB_PHYSICS
} // namespace yuugenmagan_original
#endif
