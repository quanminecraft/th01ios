void near pattern_random_purple_lasers(void)
{
	enum {
		LASER_COUNT = 10,
		INTERVAL = 10,

		KEYFRAME_0 = 50,
		KEYFRAME_1 = (KEYFRAME_0 + (LASER_COUNT * INTERVAL)),
		KEYFRAME_2 = (KEYFRAME_1 + 50),
	};

	static screen_x_t spawner_x[LASER_COUNT];
	static vram_y_t spawner_y[LASER_COUNT];

	if(boss_phase_frame < KEYFRAME_0) {
		return;
	}
	if(boss_phase_frame == KEYFRAME_0) {
		select_laser_speed_for_rank(pattern_state.speed_multiplied_by_8,
			8.5f, 9.0f, 9.5f, 10.0f
		);
		for(int i = 0; i < LASER_COUNT; i++) {
			spawner_x[i] = playfield_rand_x(0.1875f, 0.8125f);
			spawner_y[i] = playfield_rand_y((9 / 84.0f), (34 / 84.0f));
		}
	}
	if(boss_phase_frame < KEYFRAME_1) {
		int i = ((boss_phase_frame - KEYFRAME_0) / INTERVAL);
		if((boss_phase_frame % INTERVAL) != (INTERVAL - 1)) {
			int radius_cur  = (64 - (( boss_phase_frame      % INTERVAL) * 8));
			int radius_prev = (64 - (((boss_phase_frame - 1) % INTERVAL) * 8));
			if((boss_phase_frame % INTERVAL) != 0) {
				shape_circle_sloppy_unput(
					spawner_x[i], spawner_y[i], radius_prev, 1
				);
			}
			if((boss_phase_frame % INTERVAL) != (INTERVAL - 2)) {
				shape_circle_put(
					spawner_x[i], spawner_y[i], radius_cur, COL_LASER, 1
				);
			}
		} else {
			mdrv2_se_play(6);
			shootout_lasers[i].spawn(
				spawner_x[i],
				spawner_y[i],
				playfield_rand_x(),
				PLAYFIELD_BOTTOM,
				pattern_state.speed_multiplied_by_8,
				COL_LASER,
				25,
				5
			);
		}
	} else if(boss_phase_frame > KEYFRAME_2) {
		boss_phase_frame = 0;
	}
}
