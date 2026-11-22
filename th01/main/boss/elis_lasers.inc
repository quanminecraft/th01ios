int pattern_11_lasers_across(void)
{
	enum {
		INTERVAL = 10,
	};

	#define circle_center_x	form_center_x(F_GIRL)
	#define circle_center_y	(ent_still_or_wave.cur_top + (GIRL_H / 3))

	static pixel_t circle_radius;
	static bool16 direction; // ACTUAL TYPE: x_direction_t
	double target_left;
	double target_y;

	if(boss_phase_frame == 50) {
		direction = (irand() % 2);
		ent_unput_and_put_both(ent_still_or_wave, 1, C_HAND, false);
		select_laser_speed_for_rank(pattern_state.speed_multiplied_by_8,
			6.25f, 6.875f, 7.5f, 8.125f
		);
	} else if(boss_phase_frame == 60) {
		shape_circle_put(
			circle_center_x, circle_center_y, (GIRL_W / 4), COL_FX, 0x02
		);
		circle_radius = (GIRL_W / 4);
	} else if(
		(boss_phase_frame < 120) &&
		(boss_phase_frame > 60) &&
		((boss_phase_frame % 2) == 0)
	) {
		shape_circle_sloppy_unput(
			circle_center_x, circle_center_y, circle_radius, 0x02
		);
		circle_radius += 8;
		shape_circle_put(
			circle_center_x, circle_center_y, circle_radius, COL_FX, (
				(boss_phase_frame < 100) ? 0x02 :
				(boss_phase_frame < 110) ? 0x08 :
				0x20
			)
		);
	} else if(boss_phase_frame == 120) {
		shape_circle_sloppy_unput(
			circle_center_x, circle_center_y, circle_radius, 0x20
		);
	} else if(boss_phase_frame == 150) {
		ent_unput_and_put_both(ent_still_or_wave, 1, C_STILL, false);
	}

	if((boss_phase_frame >= 70) && ((boss_phase_frame % INTERVAL) == 0)) {
		if(direction == X_RIGHT) {
			target_left = (PLAYFIELD_LEFT +
				(((boss_phase_frame - 70) / INTERVAL) * (PLAYFIELD_W / 10))
			);
		} else {
			target_left = (PLAYFIELD_RIGHT -
				(((boss_phase_frame - 70) / INTERVAL) * (PLAYFIELD_W / 10))
			);
		}
		target_y = PLAYFIELD_BOTTOM;
		shootout_laser_safe(boss_phase_frame / INTERVAL).spawn(
			girl_lefteye_x(),
			girl_lefteye_y(),
			target_left,
			target_y,
			pattern_state.speed_multiplied_by_8,
			V_WHITE,
			25,
			4
		);
		mdrv2_se_play(6);

		// <= instead of < is why this pattern actually fires 11 lasers rather
		// than the 10 you might guess from looking at the target calculation
		// above.
		if(
			((direction == X_RIGHT) && (target_left >= PLAYFIELD_RIGHT)) ||
			((direction == X_LEFT) && (target_left <= PLAYFIELD_LEFT))
		) {
			boss_phase_frame = 0;
			return CHOOSE_NEW;
		}
	}
	return 1;

	#undef circle_center_y
	#undef circle_center_x
}

