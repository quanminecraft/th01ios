void pattern_four_homing_snakes(void)
{
	static Snakes<4> snakes;

	int i;
	int j;
	unsigned char angle;

	if(boss_phase_frame == 10) {
		face_expression_set_and_put(FE_GLARE);
	}
	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		for(i = 0; i < snakes.count(); i++) {
			snakes.target_locked[i] = false;
		}
		snakes_spawn_and_wobbly_aim(snakes, 0, CUP_CENTER_X, CUP_TOP, i, angle);
		for(i = 1; i < snakes.count(); i++) {
			snakes.left[i][0] = -PIXEL_NONE;
		}
		select_for_rank(pattern_state.unused, 18, 16, 14, 12);
		mdrv2_se_play(12);
		return;
	}
	if(boss_phase_frame < 400) {
		snakes_unput_update_render(i, j, angle);
		if(boss_phase_frame == 150) {
			snakes_spawn_and_wobbly_aim(
				snakes, 1, CUP_CENTER_X, CUP_TOP, i, angle
			);
			mdrv2_se_play(12);
		}
		if(boss_phase_frame == 200) {
			snakes_spawn_and_wobbly_aim(
				snakes, 2, LEFT_SLEEVE_LEFT, LEFT_SLEEVE_TOP, i, angle
			);
			mdrv2_se_play(12);
		}
		if(boss_phase_frame == 250) {
			snakes_spawn_and_wobbly_aim(
				snakes, 3, LEFT_SLEEVE_LEFT, LEFT_SLEEVE_TOP, i, angle
			);
			mdrv2_se_play(12);
		}
	} else {
		snakes_unput_all(snakes, i, j);
		boss_phase_frame = 0;
	}
}

