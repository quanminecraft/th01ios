/// Makai Stage 15 Boss - Elis
/// --------------------------

#ifdef TH01_NATIVE_ORB_PHYSICS
#include "native/elis_backend.hpp"
namespace elis_original {
#else
typedef int elis_int_t;
#include <dos.h>
#include <stdlib.h>
#include "th01/rank.h"
#include "th01/resident.hpp"
#include "th01/v_colors.hpp"
#include "th01/math/dir.hpp"
#include "th01/math/polar.hpp"
#include "th01/math/vector.hpp"
#include "th01/hardware/frmdelay.h"
#include "th01/hardware/graph.h"
#include "th01/hardware/scrollup.hpp"
#include "th01/snd/mdrv2.h"
#include "th01/formats/grc.hpp"
#include "th01/sprites/pellet.h"
#include "th01/main/shape.hpp"
#include "th01/main/hud/hp.hpp"
#include "th01/main/player/player.hpp"
#include "th01/main/boss/entity_a.hpp"
#include "th01/main/boss/defeat.hpp"
#include "th01/main/bullet/laser_s.hpp"
#include "th01/main/bullet/missile.hpp"
#include "th01/main/bullet/pellet.hpp"
#include "platform/x86real/pc98/page.hpp"

#endif
// Coordinates
// -----------

#include "th01/main/boss/elis_entities.inc"
// Form-relative coordinates
// -------------------------

// Since we can't use shot_hitbox_t() in the main function due to [form] being
// both a compile-time constant and a variable at runtime, we have to manually
// replicate it here. This is also the only reason why the shot_hitbox_(w|h)()
// macros have to exist.
#ifndef TH01_NATIVE_ORB_PHYSICS
#include "th01/main/player/shot.hpp"
#endif
#include "th01/main/boss/elis_form_coords.inc"
// Surround area
// -------------
// Common bullet spawn area around Elis.

#include "th01/main/boss/elis_surround.inc"
// .GRC entities
// -------------

#include "th01/main/boss/elis_grc.inc"
#include "th01/main/boss/elis_rifts.inc"
// Shared big circle
// -----------------
// Concrete circle structures must look like this: {
// 	unsigned char angle;
// 	int frame;
//
// 	screen_x_t center_x(void) { return …; };
// 	screen_y_t center_y(void) { return …; };
// };

#include "th01/main/boss/elis_bigcircle.inc"
// .PTN
// ----

#include "th01/main/boss/elis_background.inc"
// Patterns
// --------
static const int CHOOSE_NEW = 0;

#include "th01/main/boss/elis_star_enum.inc"
#include "th01/main/boss/elis_phase5_enum.inc"
typedef int (*elis_phase_func_t)(int id);

// Returns `CHOOSE_NEW` if done, or the pattern ID within the phase if still
// ongoing.
typedef int (*elis_phase_1_3_pattern_func_t)(void);

// Returns `SP_STAR_OF_DAVID` if done, or `SP_PATTERN` if still ongoing.
typedef elis_starpattern_ret_t (*elis_starpattern_func_t)(void);

#include "th01/main/boss/elis_pattern_state.inc"
void elis_load(void)
{
#ifdef TH01_NATIVE_ORB_PHYSICS
    native_elis_reset();native_elis_girl={};native_elis_attack={};native_elis_bat={};pattern_state={};
#endif
	pellet_interlace = true;
	elis_ent_load();
	grc_load(GRC_SLOT_BOSS_1, "boss5_gr.grc");
	ptn_new(PTN_SLOT_BG_ENT, ((GIRL_W / PTN_W) * (GIRL_H / PTN_H)));
	Missiles.load(PTN_SLOT_MISSILE);
	void elis_setup(void);
	elis_setup();
}

#include "th01/main/boss/elis_setup.inc"
void elis_free(void)
{
	elis_ent_free();
	grc_free(GRC_SLOT_BOSS_1);
	ptn_free(PTN_SLOT_BG_ENT);
	ptn_free(PTN_SLOT_MISSILE);
}

#include "th01/main/boss/elis_teleport.inc"
#ifndef TH01_NATIVE_ORB_PHYSICS
#include "th01/main/select_r.cpp"
#endif

#include "th01/main/boss/elis_lasers.inc"
#include "th01/main/boss/elis_rift_missiles.inc"
#include "th01/main/boss/elis_circle_pellets.inc"
#include "th01/main/boss/elis_star_lines.inc"
int phase_1(int id)
{
	switch(id) {
	case CHOOSE_NEW:
		// Note that this includes `CHOOSE_NEW`. Due to how phase_frame_common()
		// switches between patterns and teleporting, this adds a 25% chance of
		// Elis skipping an attack cycle and teleporting again.
		return (irand() % 4);
	case 1: return pattern_11_lasers_across();
	case 2: return pattern_random_downwards_missiles();
	case 3: return pattern_pellets_along_circle();
	}
	return CHOOSE_NEW;
}

#include "th01/main/boss/elis_star.inc"
#include "th01/main/boss/elis_curved_rings.inc"
#include "th01/main/boss/elis_sphere_clusters.inc"
#include "th01/main/boss/elis_rift_pellets.inc"
#include "th01/main/boss/elis_phase3.inc"
#include "th01/main/boss/elis_transform.inc"
// Like the flystep functions in later games, just without the "step" part, and
// with explicit pattern/transformation semantics.
#include "th01/main/boss/elis_bat_flight.inc"
#include "th01/main/boss/elis_bat_attacks.inc"
#include "th01/main/boss/elis_symmetric.inc"
#include "th01/main/boss/elis_safety_circle.inc"
#include "th01/main/boss/elis_final_lasers.inc"
#include "th01/main/boss/elis_phase5.inc"
#ifdef TH01_NATIVE_ORB_PHYSICS
// Preserve the original call's right-to-left argument evaluation explicitly.
static bool16 native_elis_random_teleport(){
    screen_y_t top=elis_playfield_random_top();
    screen_x_t left=elis_playfield_random_left();
    return wave_teleport(left,top);
}
#endif
void elis_main(void)
{
	static elis_form_t form = F_GIRL;
	static struct {
		elis_int_t invincibility_frame;
		bool16 invincible;

		#define hit_update_and_render(form_inlined, flash_colors) { \
			boss_hit_update_and_render( \
				hit.invincibility_frame, \
				hit.invincible, \
				boss_hp, \
				flash_colors, \
				(sizeof(flash_colors) / sizeof(flash_colors[0])), \
				7000, \
				(form == F_GIRL) \
					? ent_still_or_wave.hittest_orb() \
					: ent_bat.hittest_orb(), \
				form_shot_hitbox_left(form_inlined), \
				form_shot_hitbox_top(form_inlined), \
				form_shot_hitbox_w(form_inlined), \
				form_shot_hitbox_h(form_inlined) \
			); \
		}
	} hit;
	static struct {
		union {
			elis_int_t pattern;

			// ZUN bloat: The static one would have been fine.
			elis_form_t form;
		} cur;
		bool16 teleport_done;

		void frame_common(int8_t phase_id) {
			boss_phase_frame++;
			hit.invincibility_frame++;

			// ZUN bloat: Already done as part of ent_unput_and_put_both(),
			// which is the only function that reads from [ent_attack].
			if(phase_id != 1) {
				ent_attack.pos_cur_set_to(ent_still_or_wave);
			}
		}

#ifdef TH01_NATIVE_ORB_PHYSICS
		#define phase_frame_common(phase_id, phase_func, flash_colors) { \
			phase.frame_common(phase_id); \
			\
			if(phase.cur.pattern != CHOOSE_NEW) { \
				phase.cur.pattern = phase_func(phase.cur.pattern); \
			} else if(phase.teleport_done == false) { \
				phase.teleport_done = native_elis_random_teleport(); \
			} else if(phase.teleport_done == true) { \
				phase.cur.pattern = phase_func(phase.cur.pattern); \
				phase.teleport_done = false; \
			} \
			hit_update_and_render(F_GIRL, flash_colors); \
		}

#else
		#define phase_frame_common(phase_id, phase_func, flash_colors) { \
			phase.frame_common(phase_id); \
			\
			if(phase.cur.pattern != CHOOSE_NEW) { \
				phase.cur.pattern = phase_func(phase.cur.pattern); \
			} else if(phase.teleport_done == false) { \
				phase.teleport_done = wave_teleport( \
					elis_playfield_random_left(), elis_playfield_random_top() \
				); \
			} else if(phase.teleport_done == true) { \
				phase.cur.pattern = phase_func(phase.cur.pattern); \
				phase.teleport_done = false; \
			} \
			hit_update_and_render(F_GIRL, flash_colors); \
		}

#endif
		#define phase_done(end_hp) ( \
			(boss_hp <= end_hp) && (phase.teleport_done == true) \
		)

		void next(int8_t phase_new, elis_int_t pattern_new) {
			boss_phase = phase_new;
			boss_phase_frame = 0;
			hit.invincibility_frame = 0;
			cur.pattern = pattern_new;
			teleport_done = false;
		}
	} phase;
	static pixel_t bat_velocity_y;
	static pixel_t bat_velocity_x;
	static bool initial_hp_rendered;

	screen_x_t head_left;
	screen_y_t head_top;
	bool16 trails_offscreen;
	const vc_t flash_colors[] = { 3, 6, 8, 2 };
	unsigned char angle;

	#ifdef TH01_NATIVE_ORB_PHYSICS
    if(!native_elis_before_frame())return;
    if(native_elis_take_reset()){form=F_GIRL;hit={};phase={};bat_velocity_x=bat_velocity_y=0;initial_hp_rendered=false;}
#endif
    Missiles.unput_update_render();

	// Entrance animation
	if(boss_phase == 0) {
#include "th01/main/boss/elis_entrance.inc"
	} else if(boss_phase == 1) {
		// ZUN bug: Since the fight only ends in Phase 5 at the earliest, HP
		// subtraction in debug mode can lead to this function being called
		// with [boss_phase_frame] being larger than the initial HP value,
		// causing the heap corruption bug mentioned in the comment of this
		// function. Elis starts with an even number of total HP, so this will
		// even happen for the easiest possible case of holding ↵ Return for
		// the first 8 frames of phase 1.
		hud_hp_increment_render(initial_hp_rendered, boss_hp, boss_phase_frame);

		phase_frame_common(1, phase_1, flash_colors);
		if(!hit.invincible && phase_done(HP_PHASE_1_END)) {
			phase.next(2, CHOOSE_NEW);
		}
	} else if(boss_phase == 2) {
		boss_phase_frame++;
		phase.teleport_done = wave_teleport(BASE_LEFT, BASE_TOP);
		if(phase.teleport_done == true) {
			phase_3(99);

			phase.next(3, 1);
		}
	} else if(boss_phase == 3) {
		phase_frame_common(3, phase_3, flash_colors);
		if(!hit.invincible && phase_done(HP_PHASE_3_END)) {
			phase.next(4, CHOOSE_NEW);
			form = F_GIRL;
		}
	} else if(boss_phase == 4) {
		boss_phase_frame++;
		if(!phase.teleport_done) {
			phase.teleport_done = wave_teleport(BASE_LEFT, BASE_TOP);
		} else {
			phase.cur.form = transform_girl_to_bat();
		}
		if(phase.cur.form == F_BAT) {
			phase.next(5, CHOOSE_NEW);
			form = F_BAT;

			phase_5(form, bat_velocity_x, bat_velocity_y, true);
		}
	} else if(boss_phase == 5) {
		phase.frame_common(5);
#include "th01/main/boss/elis_bat_move.inc"
		phase_5(form, bat_velocity_x, bat_velocity_y);
		hit_update_and_render(form, flash_colors);
		if(boss_hp <= HP_PHASE_5_END) {
			elis_int_t i;

			mdrv2_bgm_fade_out_nonblock();
			Pellets.unput_and_reset_nonclouds();
			girl_bg_put(1);
			Missiles.reset();
			shootout_lasers_unput_and_reset_broken(i, SHOOTOUT_LASER_COUNT);
			boss_defeat_animate();
		}
	}

	#undef phase_done
	#undef phase_frame_common
	#undef hit_update_and_render
#ifdef TH01_NATIVE_ORB_PHYSICS
    native_elis_observe(form==F_BAT,phase.cur.pattern,hit.invincibility_frame);
#endif
}

#ifdef TH01_NATIVE_ORB_PHYSICS
} // namespace elis_original
#endif
