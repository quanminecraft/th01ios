void pattern_lasers_and_3_spread(void)
{
	// These have no reason to be static.
	static screen_x_t target_left;
	static screen_y_t target_y;

	static bool16 right_to_left;

	enum {
		INTERVAL = 10,
	};

	if(boss_phase_frame == 10) {
		face_expression_set_and_put(FE_AIM);
	}
	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		right_to_left = (irand() % 2);

		// Divisor = number of lasers that are effectively fired.
		select_for_rank(pattern_state.delta_x,
			(PLAYFIELD_W / 5),
			(PLAYFIELD_W / 6.66),
			(PLAYFIELD_W / 8),
			(PLAYFIELD_W / 10)
		);
	}
	if((boss_phase_frame % INTERVAL) == 0) {
		if(right_to_left == 0) {
			target_left = (PLAYFIELD_LEFT + (
				((boss_phase_frame - 100) / INTERVAL) * pattern_state.delta_x
			));
		} else {
			target_left = (PLAYFIELD_RIGHT - (
				((boss_phase_frame - 100) / INTERVAL) * pattern_state.delta_x
			));
		}
		target_y = PLAYFIELD_BOTTOM;

		shootout_laser_safe(boss_phase_frame / INTERVAL).spawn(
			SWORD_CENTER_X, SWORD_CENTER_Y,
			target_left, target_y,
			(to_sp(8.5f) / 2), V_WHITE, 30, 5
		);
		mdrv2_se_play(6);

		if(
			((right_to_left == false) && (target_left >= PLAYFIELD_RIGHT)) ||
			((right_to_left == true)  && (target_left <= PLAYFIELD_LEFT))
		) {
			boss_phase_frame = 0;
		}

		Pellets.add_group(
			SWORD_CENTER_X, SWORD_CENTER_Y, PG_3_SPREAD_WIDE_AIMED, to_sp(4.5f)
		);
	}
}

