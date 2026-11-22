void pattern_halfcircle_missiles_downwards_from_corners(void)
{
	pixel_t velocity_x;
	pixel_t velocity_y;
	static unsigned char missile_angle;

	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		sq.init(SF_REGULAR);
		missile_angle = 0x00;

		// MODDERS: Just use regular subpixels. They perfectly support a
		// fraction of .5… especially if ZUN chops off the fractional digits
		// when reading this variable anyway. :(
		// So, the actual speeds are (4, 4, 5, 5).
		select_for_rank(pattern_state.speed_decimal.v,
			to_dsp(4.0f), to_dsp(4.5f), to_dsp(5.0f), to_dsp(5.5f)
		);
		mdrv2_se_play(8);
	}
	if((boss_phase_frame % SQUARE_INTERVAL) == 0) {
		sq.set_and_unput();
		sq.angle -= 0x0C;
		if(sq.radius < SEAL_RADIUS) {
			sq.radius += SQUARE_RADIUS_STEP;
		} else if((boss_phase_frame > 180) && ((boss_phase_frame % 16) == 8)) {
			// Same corner coordinate quirk as seen in the first pattern.

			vector2(
				velocity_x,
				velocity_y,
				pattern_state.speed_decimal.to_pixel(), // :(
				missile_angle
			);

			for(int i = 0; i < SQUARE_POINTS; i++) {
				Missiles.add(
					sq.corners.c.regular[i].x,
					sq.corners.c.regular[i].y,
					velocity_x,
					velocity_y
				);
			}
			mdrv2_se_play(6);
			missile_angle += 0x0D;
		}
		sq.set_and_put();
	}
	if(boss_phase_frame > 340) {
		sq.deactivate();
		boss_phase_frame = 0;
	}
}

void pattern_slow_pellet_spray_from_corners(void)
{
	enum {
		KEYFRAME_SQUARE = 100,
		KEYFRAME_FIRE_LEFT_TO_RIGHT = 180,
		KEYFRAME_FIRE_RIGHT_TO_LEFT = 270,
		KEYFRAME_DONE = 370,
	};

	static unsigned char pellet_angle;

	if(boss_phase_frame < KEYFRAME_SQUARE) {
		return;
	}
	if(boss_phase_frame == KEYFRAME_SQUARE) {
		sq.init(SF_REGULAR);
		pellet_angle = 0x80;
		select_subpixel_for_rank(pattern_state.speed, 2.0f, 2.5f, 3.0f, 3.5f);
		mdrv2_se_play(8);
	}
	if((boss_phase_frame % SQUARE_INTERVAL) == 0) {
		sq.set_and_unput();
		sq.angle = ((boss_phase_frame > KEYFRAME_FIRE_RIGHT_TO_LEFT)
			? (sq.angle + 0x0C)
			: (sq.angle - 0x0C)
		);
		if(sq.radius < SEAL_RADIUS) {
			sq.radius += SQUARE_RADIUS_STEP;
		} else if(boss_phase_frame > KEYFRAME_FIRE_LEFT_TO_RIGHT) {
			// Same corner coordinate quirk as seen in the first pattern.

			pellet_angle = ((boss_phase_frame > KEYFRAME_FIRE_RIGHT_TO_LEFT)
				? (pellet_angle + 0x0C)
				: (pellet_angle - 0x0C) // slightly overshooting the half circle
			);
			for(int i = 0; i < SQUARE_POINTS; i++) {
				Pellets.add_single(
					sq.corners.c.regular[i].x,
					sq.corners.c.regular[i].y,
					pellet_angle,
					pattern_state.speed
				);
			}
			mdrv2_se_play(6);
		}
		sq.set_and_put();
	}
	if(boss_phase_frame > KEYFRAME_DONE) {
		sq.deactivate();
		boss_phase_frame = 0;
	}
}

void pattern_aimed_lasers_from_corners(void)
{
	enum {
		LASER_W = 4,
	};

	if(boss_phase_frame == 50) {
		mima_put_cast_both();
	}
	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		sq.init(SF_REGULAR);
		select_laser_speed_for_rank(pattern_state.speed_multiplied_by_8,
			6.25f, 6.75f, 7.25f, 7.75f
		);
		mdrv2_se_play(8);
	}
	if((boss_phase_frame % SQUARE_INTERVAL) == 0) {
		sq.set_and_unput();
		sq.angle += 0x03;
		if(sq.radius < SEAL_RADIUS) {
			sq.radius += SQUARE_RADIUS_STEP;
		}

		// ... especially when remembering the coordinates for future frames
		// is exactly what fixes the corner coordinate quirk in this pattern.
		sq.set_and_put();
	}
	if((boss_phase_frame > 180) && (boss_phase_frame < 300)) {
		int i = (boss_phase_frame % SQUARE_POINTS);
		shootout_lasers[i].spawn(
			sq.corners.c.regular[i].x,
			sq.corners.c.regular[i].y,
			(player_center_x() - (LASER_W / 2)),
			player_bottom(),
			pattern_state.speed_multiplied_by_8,
			V_WHITE,
			20,
			LASER_W
		);
	}
	if(boss_phase_frame > 300) {
		sq.deactivate();
		boss_phase_frame = 0;
	}
}

