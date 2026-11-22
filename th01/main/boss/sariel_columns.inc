void near pattern_vertical_stacks_from_bottom_then_random_rain_from_top(void)
{
	enum {
		COLUMN_INTERVAL = 10,
		COLUMN_COUNT = 20,
		SPAWNRAY_VELOCITY_X = (PLAYFIELD_W / 80),
		COLUMN_DISTANCE_X = (PLAYFIELD_W / COLUMN_COUNT),

		KEYFRAME_0 = 50,
		KEYFRAME_1 = (KEYFRAME_0 + (PLAYFIELD_W / SPAWNRAY_VELOCITY_X)),
		KEYFRAME_2 = (KEYFRAME_1 + 20),
		KEYFRAME_3 = (KEYFRAME_2 + (COLUMN_COUNT * COLUMN_INTERVAL)),
		KEYFRAME_4 = (KEYFRAME_3 + (COLUMN_COUNT * COLUMN_INTERVAL)),
		KEYFRAME_5 = (KEYFRAME_4 + 50),
	};

	static SymmetricSpawnraysWithDebris<
		PLAYFIELD_CENTER_X, FACE_CENTER_Y, (DEBRIS_W / 4)
	> rays;

	// ZUN bug: Leaving this uninitialized indeed implies vortex sprites for
	// the first 5 frames, until this actually reaches C_DEBRIS...
	static vortex_or_debris_cel_t debris_cel;

	unsigned char angle;

	if(boss_phase_frame < KEYFRAME_0) {
		return;
	}
	if(boss_phase_frame == KEYFRAME_0) {
		// Note that the value for Lunatic is less than the COLUMN_INTERVAL.
		// This will move the spawners into another VRAM row before the pattern
		// ends.
		select_for_rank(pattern_state.interval, 16, 13, 10, 8);

		rays.target_l_x = 2;
		rays.target_y = (PLAYFIELD_BOTTOM - 1);
	} else if(boss_phase_frame < KEYFRAME_1) {
		if((boss_phase_frame % 2) == 0) {
			rays.unput_and_put(X_LEFT, debris_cel, SPAWNRAY_VELOCITY_X);
		}
		if((boss_phase_frame % 2) == 1) {
			rays.unput_and_put(X_RIGHT, debris_cel, SPAWNRAY_VELOCITY_X);
			debris_cel = debris_cel_next(debris_cel);
		}
		rays.target_l_x += SPAWNRAY_VELOCITY_X;
	} else if(boss_phase_frame == KEYFRAME_1) {
		rays.ray_unput_right(SPAWNRAY_VELOCITY_X);
		rays.target_l_x = PLAYFIELD_LEFT;
	} else if(
		(boss_phase_frame > KEYFRAME_2) &&
		(boss_phase_frame < KEYFRAME_3)
	) {
		if((boss_phase_frame % COLUMN_INTERVAL) == 0) {
			for(subpixel_t i = to_sp(0.0f); i < to_sp(4.0f); i += to_sp(1.0f)) {
				// How nice!
				if(abs(player_center_x() - rays.target_x(X_LEFT)) > PLAYER_W) {
					Pellets.add_single(
						rays.target_x(X_LEFT),
						(rays.target_y - PELLET_H),
						-0x40,
						(i + to_sp(3.0f))
					);
				}
				if(abs(player_center_x() - rays.target_x(X_RIGHT)) > PLAYER_W) {
					Pellets.add_single(
						rays.target_x(X_RIGHT),
						(rays.target_y - PELLET_H),
						-0x40,
						(i + to_sp(3.0f))
					);
				}
			}
			rays.target_l_x += COLUMN_DISTANCE_X;
			mdrv2_se_play(7);
		}
	} else if(boss_phase_frame == KEYFRAME_3) {
		rays.target_l_x = PLAYFIELD_LEFT;
		rays.target_y = PLAYFIELD_TOP;
	} else if(boss_phase_frame < KEYFRAME_4) {
		if((boss_phase_frame % pattern_state.interval) == 0) {
			for(int i = 0; i < 2; i++) {
				rays.fire_random_to_bottom(angle, X_LEFT, to_sp(3.0f));
				rays.fire_random_to_bottom(angle, X_RIGHT, to_sp(3.0f));
			}
			rays.target_l_x += COLUMN_DISTANCE_X;
		}
	} else if(boss_phase_frame > KEYFRAME_5) {
		boss_phase_frame = 0;
	}
}
