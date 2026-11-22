#define miss_effect_put(left, frame) \
	ptn_put_8(left, player_top, (PTN_MISS_EFFECT + (frame % MISS_EFFECT_CELS)))

void player_miss_animate_and_update(void)
{
	int frame;
	screen_x_t effect_left;
	screen_x_t moveout_target_right;
	screen_x_t moveout_target_left;
	int prev_bombs;

	// Miss sprite and shake
	// ---------------------

	// ZUN bug: This should have been the 48×48 unblitting call that's done
	// after the shake. It's easily possible to get hit during an 48×48
	// animation, which will end up leaving the extra pixels on screen during
	// the next 16 frames.
	ptn_sloppy_unput_16(player_left, player_top);

	player_miss_put();

	for(frame = 0; frame < 16; frame++) {
		z_vsync_wait_and_scrollup(RES_Y - ((frame % 2) * 8));
		frame_delay(2);
	}
	z_vsync_wait_and_scrollup(0);
	// ---------------------

	egc_copy_rect_1_to_0_16(
		player_48_left_for(player_left), player_48x48_top(), 48, 48
	);

	Shots.unput_and_reset();
	Pellets.decay();
	player_reset();

	// Effect
	// ------
	enum {
		MOVEOUT_FRAMES = 10,
		MOVEIN_FRAMES = 20,
		FRAMES = (MOVEOUT_FRAMES + MOVEIN_FRAMES),

		MOVEOUT_DISTANCE_X = (PLAYFIELD_W / 4),
		MOVEOUT_DISTANCE_PER_FRAME_X = (MOVEOUT_DISTANCE_X / MOVEOUT_FRAMES),
	};

	moveout_target_right = (player_left + MOVEOUT_DISTANCE_X);
	if(moveout_target_right >= PLAYER_LEFT_MAX) {
		moveout_target_right = PLAYER_LEFT_MAX;
	}
	moveout_target_left = (player_left - MOVEOUT_DISTANCE_X);
	if(moveout_target_left < PLAYER_LEFT_MIN) {
		moveout_target_left = PLAYER_LEFT_MIN;
	}

	effect_left = player_left;
	for(frame = 0; frame < FRAMES; frame++) {
		ptn_sloppy_unput_16(player_left, player_top);
		ptn_sloppy_unput_16(effect_left, player_top);

		player_left += (frame < MOVEOUT_FRAMES)
			? +MOVEOUT_DISTANCE_PER_FRAME_X
			: ((PLAYER_LEFT_START - moveout_target_right) / MOVEIN_FRAMES);
		effect_left += (frame < MOVEOUT_FRAMES)
			? -MOVEOUT_DISTANCE_PER_FRAME_X
			: ((PLAYER_LEFT_START - moveout_target_left) / MOVEIN_FRAMES);

		if(player_left >= PLAYER_LEFT_MAX) {
			player_left = PLAYER_LEFT_MAX;
		}
		if(effect_left < PLAYER_LEFT_MIN) {
			effect_left = PLAYER_LEFT_MIN;
		}

		// We might have accidentally unblitted it earlier, after all.
		orb_put_default();

		miss_effect_put(player_left, frame);
		miss_effect_put(effect_left, frame);
		frame_delay((frame < 6) ? 1 : (((FRAMES - frame) / 10) + 1));
	}

	ptn_sloppy_unput_16(player_left, player_top);
	ptn_sloppy_unput_16(effect_left, player_top);
	// ------

	hud_lives_put(rem_lives + 1);

	prev_bombs = rem_bombs;
	rem_bombs = (BOMBS_EXTRA_PER_LIFE_LOST + rem_bombs);
	if(rem_bombs > BOMBS_MAX) {
		rem_bombs = BOMBS_MAX;
	} else {
		hud_bombs_put(prev_bombs);
	}
	timer_extend_and_put();
	pellet_speed_lower(0.0f, -0.05f);
}
