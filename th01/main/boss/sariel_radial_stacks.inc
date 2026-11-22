void near pattern_radial_stacks_and_lasers(void)
{
	#define CENTER_X	PLAYFIELD_CENTER_X
	// Technically wrong as the ray's origin point, but who cares.
	#define CENTER_Y	(SHIELD_CENTER_Y - (PELLET_H / 2))

	static unsigned char angle;
	static unsigned char angle_velocity;

	screen_x_t target_x;
	screen_y_t target_y;

	if(boss_phase_frame < 5) {
		spawnray_reset();
	}
	if(boss_phase_frame < 150) {
		return;
	} else if(boss_phase_frame == 150) {
		angle = 0x00;
		angle_velocity = 0x01;
		select_laser_speed_for_rank(pattern_state.speed_multiplied_by_8,
			7.5f, 8.125f, 8.75f, 9.375f
		);
		mdrv2_se_play(6);
	} else if(boss_phase_frame < 200) {
		target_x = polar_x(CENTER_X, SEAL_RADIUS, angle);
		target_y = polar_y(CENTER_Y, SEAL_RADIUS, angle);
		spawnray_unput_and_put(CENTER_X, CENTER_Y, target_x, target_y, V_WHITE);
		angle += angle_velocity;
		angle_velocity++;
		if((boss_phase_frame % 10) == 0) {
			mdrv2_se_play(6);
		}
	} else if(boss_phase_frame == 200) {
		target_x = -PIXEL_NONE;
		target_y = -PIXEL_NONE;
		spawnray_unput_and_put(CENTER_X, CENTER_Y, target_x, target_y, V_WHITE);
		angle = 0x00;
	} else if(boss_phase_frame <= 400) {
		if((boss_phase_frame % 5) == 0) {
			target_x = polar_x(CENTER_X, 600, angle);
			target_y = polar_y(CENTER_Y, 600, angle);
			mdrv2_se_play(7);
			if((boss_phase_frame % 15) == 0) {
				shootout_laser_safe((boss_phase_frame - 215) / 15).spawn(
					CENTER_X, CENTER_Y, target_x, target_y,
					pattern_state.speed_multiplied_by_8, V_WHITE, 20, 4
				);
			} else {
				Pellets.add_single(CENTER_X, CENTER_Y, angle, to_sp(6.25f));
			}
			Pellets.add_single(CENTER_X, CENTER_Y, (angle - 0x04), to_sp(6.0f));
			Pellets.add_single(CENTER_X, CENTER_Y, (angle + 0x04), to_sp(6.0f));
			if(rank >= RANK_HARD) {
				Pellets.add_single(
					CENTER_X, CENTER_Y, (angle - 0x04), to_sp(4.0f)
				);
				Pellets.add_single(
					CENTER_X, CENTER_Y, (angle + 0x04), to_sp(4.0f)
				);
				if(rank == RANK_LUNATIC) {
					Pellets.add_single(CENTER_X, CENTER_Y, angle, to_sp(4.0f));
					Pellets.add_single(CENTER_X, CENTER_Y, angle, to_sp(3.0f));
				}
			}
			angle += 0x0C;
		}
	} else if(boss_phase_frame == 450) {
		boss_phase_frame = 0;
	}

	#undef CENTER_Y
	#undef CENTER_X
}
