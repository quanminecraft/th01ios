inline void swordray_unput_put_and_move(
	screen_x_t& end_x, screen_x_t& end_y, screen_x_t delta_x, screen_y_t delta_y
) {
	spawnray_unput_and_put(SWORD_CENTER_X, SWORD_CENTER_Y, end_x, end_y, 6);
	// Gimme those original instructions!
	if(delta_x < 0) { end_x -= -delta_x; } else { end_x += delta_x; }
	if(delta_y < 0) { end_y -= -delta_y; } else { end_y += delta_y; }
}

inline void swordray_unput(const screen_x_t& end_x, const screen_x_t& end_y) {
	graph_r_line_unput(SWORD_CENTER_X, SWORD_CENTER_Y, end_x, end_y);
}

void pattern_rain_from_edges(void)
{
	static screen_x_t end_x;
	static screen_y_t end_y;
	static int unused;

	enum {
		SPAWNRAY_SPEED = 8,
		FRAMES_VERTICAL = 25,
		FRAMES_HORIZONTAL = (PLAYFIELD_W / SPAWNRAY_SPEED),

		KEYFRAME_0 = 100,
		KEYFRAME_1 = (KEYFRAME_0 + FRAMES_VERTICAL), // up
		KEYFRAME_2 = (KEYFRAME_1 + FRAMES_HORIZONTAL), // right
		KEYFRAME_3 = (KEYFRAME_2 + FRAMES_VERTICAL), // down
		KEYFRAME_4 = (KEYFRAME_3 + FRAMES_VERTICAL), // up
		KEYFRAME_5 = (KEYFRAME_4 + FRAMES_HORIZONTAL), // left
		KEYFRAME_6 = (KEYFRAME_5 + FRAMES_VERTICAL), // down
	};

	if(boss_phase_frame == 10) {
		face_expression_set_and_put(FE_AIM);
	}
	if(boss_phase_frame < KEYFRAME_0) {
		return;
	}
	if(boss_phase_frame == KEYFRAME_0) {
		end_x = PLAYFIELD_LEFT;
		end_y = (PLAYFIELD_TOP + (SPAWNRAY_SPEED * FRAMES_VERTICAL));
		unused = 1;
		select_for_rank(pattern_state.interval, 5, 3, 2, 2);
	}
	if(boss_phase_frame < KEYFRAME_1) {
		swordray_unput_put_and_move(end_x, end_y, 0, -SPAWNRAY_SPEED);
	} else if(boss_phase_frame == KEYFRAME_1) {
		end_x = PLAYFIELD_LEFT;
		end_y = PLAYFIELD_TOP;
		swordray_unput_put_and_move(end_x, end_y, +SPAWNRAY_SPEED, 0);

		unused = 0;
	} else if(boss_phase_frame < KEYFRAME_2) {
		swordray_unput_put_and_move(end_x, end_y, +SPAWNRAY_SPEED, 0);
	} else if(boss_phase_frame == KEYFRAME_2) {
		end_x = (PLAYFIELD_RIGHT - 1);
		end_y = PLAYFIELD_TOP;
		swordray_unput_put_and_move(end_x, end_y, 0, +SPAWNRAY_SPEED);

		unused = 2;
	} else if(boss_phase_frame < KEYFRAME_3) {
		swordray_unput_put_and_move(end_x, end_y, 0, +SPAWNRAY_SPEED);
	} else if(boss_phase_frame == KEYFRAME_3) {
		end_x = (PLAYFIELD_RIGHT - 1);
		end_y = (PLAYFIELD_TOP + (SPAWNRAY_SPEED * FRAMES_VERTICAL));
		swordray_unput_put_and_move(end_x, end_y, 0, -SPAWNRAY_SPEED);

		unused = 2;
	} else if(boss_phase_frame < KEYFRAME_4) {
		swordray_unput_put_and_move(end_x, end_y, 0, -SPAWNRAY_SPEED);
	} else if(boss_phase_frame == KEYFRAME_4) {
		end_x = (PLAYFIELD_RIGHT - 1);
		end_y = PLAYFIELD_TOP;
		swordray_unput_put_and_move(end_x, end_y, -SPAWNRAY_SPEED, 0);

		unused = 0;
	} else if(boss_phase_frame < KEYFRAME_5) {
		swordray_unput_put_and_move(end_x, end_y, -SPAWNRAY_SPEED, 0);
	} else if(boss_phase_frame == KEYFRAME_5) {
		end_x = PLAYFIELD_LEFT;
		end_y = PLAYFIELD_TOP;
		swordray_unput_put_and_move(end_x, end_y, 0, +SPAWNRAY_SPEED);

		unused = 1;
	} else if(boss_phase_frame < KEYFRAME_6) {
		swordray_unput_put_and_move(end_x, end_y, 0, +SPAWNRAY_SPEED);
	} else if(boss_phase_frame == KEYFRAME_6) {
		// Wait, what, changing the end point of the ray immediately before
		// unblitting?! Technically wrong, but since line unblitting uses
		// 32-dot chunks anyway, this doesn't leave any visible glitches.
		end_y -= SPAWNRAY_SPEED;
		swordray_unput(end_x, end_y);
		boss_phase_frame = 0;
	}
	if((boss_phase_frame % 10) == 0) {
		mdrv2_se_play(6);
	}
	if((boss_phase_frame % pattern_state.interval) == 0) {
		pellets_add_single_rain(end_x, end_y, (irand() & 0x7F), 2.0f);
		pellets_add_single_rain(end_x, end_y, (irand() & 0x7F), 2.0f);
	}
}

