int pattern_clusters_from_spheres(void)
{
	enum {
		INTERVAL = 10,
		SPHERE_COUNT = 10,
		SPHERE_FRAMES = (SPHERE_COUNT * INTERVAL),

		// Spheres spawned, start firing
		KEYFRAME_FIRE = SPHERE_FRAMES,

		// Spheres start being removed
		KEYFRAME_REMOVE = (KEYFRAME_FIRE + 30),

		// All spheres fired
		KEYFRAME_FIRE_DONE = (KEYFRAME_FIRE + SPHERE_FRAMES),

		// All spheres removed
		KEYFRAME_REMOVE_DONE = (KEYFRAME_REMOVE + SPHERE_FRAMES),

		KEYFRAME_DONE = (KEYFRAME_REMOVE_DONE + 20),
	};

	struct Spheres : public EntitiesTopleft<SPHERE_COUNT> {
		static pixel_t column_w(screen_x_t edge) {
			return ((edge - ent_still_or_wave.cur_left) / count());
		}

		static pixel_t row_h(screen_y_t edge) {
			return (
				(((irand() % edge) + edge) - ent_still_or_wave.cur_top) / count()
			);
		}
	};

	static Spheres spheres;
	int i;

	ent_attack_render();
	if(
		(boss_phase_frame <= KEYFRAME_FIRE) &&
		((boss_phase_frame % INTERVAL) == 0)
	) {
		#define sphere_i (boss_phase_frame / INTERVAL)

		if(ent_still_or_wave.cur_left < BASE_LEFT) {
			spheres.left[sphere_i - 1] = (
				(sphere_i * spheres.column_w(PLAYFIELD_RIGHT - SPHERE_W)) +
				(form_center_x(F_GIRL) - (SPHERE_W / 2))
			);
		} else {
			spheres.left[sphere_i - 1] = (
				(sphere_i * spheres.column_w(PLAYFIELD_LEFT)) +
				(form_center_x(F_GIRL) - (SPHERE_W / 2))
			);
		}
		spheres.top[sphere_i - 1] = (
			(sphere_i * spheres.row_h(playfield_fraction_y(25 / 42.0f))) +
			(form_center_y(F_GIRL) - (SPHERE_H / 2))
		);

		// We might have unblitted some of the earlier sprites during the
		// previous interval...
		for(i = 0; i < sphere_i; i++) {
			elis_grc_put(spheres, i, C_SPHERE_LARGE, COL_FX);
		}

		#undef sphere_i
	} else if(
		(boss_phase_frame > KEYFRAME_FIRE) &&
		(boss_phase_frame <= KEYFRAME_REMOVE_DONE) &&
		((boss_phase_frame % INTERVAL) == 0)
	) {
		if(boss_phase_frame <= KEYFRAME_FIRE_DONE) {
			#define sphere_i ((boss_phase_frame - KEYFRAME_FIRE) / INTERVAL)

			mdrv2_se_play(7);

			// ZUN bug: This will only really be visible in the bad case where
			// the sphere has been accidentally unblitted before. In the usual
			// case, the rift sprite will be blitted *on top* of the circle
			// sprite at the same position, which only adds a total of 5 new
			// blue pixels outside of the circle.
			elis_grc_put(spheres, (sphere_i - 1), C_RIFT, COL_FX);

			for(i = sphere_i; i < SPHERE_COUNT; i++) {
				elis_grc_put(spheres, i, C_SPHERE_LARGE, COL_FX);
			}

			select_for_rank(pattern_state.count, 5, 7, 9, 13);
			for(i = 0; i < pattern_state.count; i++) {
				subpixel_t speed;
				unsigned char angle;

				angle = (irand() & 0x7F);
				speed = ((irand() + to_sp(1.5f)) & (to_sp(4.0f) - 1));
				Pellets.add_single(
					spheres.left[sphere_i - 1],
					spheres.top[sphere_i - 1],
					angle,
					speed
				);
			}

			#undef sphere_i
		}
		if(boss_phase_frame > KEYFRAME_REMOVE) {
			#define sphere_i ((boss_phase_frame - KEYFRAME_REMOVE) / INTERVAL)
			elis_grc_sloppy_unput(spheres, (sphere_i - 1));
			#undef sphere_i
		}
	} else if(boss_phase_frame > KEYFRAME_DONE) {
		boss_phase_frame = 0;
		return CHOOSE_NEW;
	}
	return 2;
}

