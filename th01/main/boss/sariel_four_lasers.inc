void near pattern_four_aimed_lasers(void)
{
	enum {
		ORIGIN_DISTANCE_X_1 = (SEAL_CENTER_X - SEAL_RADIUS),
		ORIGIN_Y_1 = FACE_CENTER_Y,
		ORIGIN_Y_2 = SHIELD_CENTER_Y,
	};

	static screen_x_t origin_x;
	static screen_y_t origin_y;
	static struct {
		pixel_t velocity_y;
		pixel_t velocity_x;

		void unput_update_render(void) {
			spawnray_unput_and_put(
				PLAYFIELD_CENTER_X, ORIGIN_Y_1, origin_x, origin_y, V_WHITE
			);
			origin_x += velocity_x;
			origin_y += velocity_y;
		}
	} spawnray;
	static screen_x_t target_first;

	#define spawnray_init(origin_x_, origin_y_, target_left) { \
		origin_x = origin_x_; \
		origin_y = origin_y_; \
		vector2_between( \
			origin_x, \
			origin_y, \
			target_left, \
			PLAYFIELD_BOTTOM, \
			spawnray.velocity_x, \
			spawnray.velocity_y, \
			16 \
		); \
	}

	#define fire(laser_id, origin_left, origin_top, target_left) { \
		shootout_lasers[laser_id].spawn( \
			origin_left, origin_top, target_left, PLAYFIELD_BOTTOM, \
			pattern_state.speed_multiplied_by_8, V_WHITE, 50, 8 \
		); \
		mdrv2_se_play(6); \
	}

	if(boss_phase_frame < 5) {
		spawnray_reset();
	}

	if(boss_phase_frame < 50) {
		return;
	} else if(boss_phase_frame == 50) {
		spawnray_init(
			(PLAYFIELD_RIGHT - ORIGIN_DISTANCE_X_1), ORIGIN_Y_1, player_left
		);

		target_first = player_left;
		select_laser_speed_for_rank(pattern_state.speed_multiplied_by_8,
			7.5f, 8.0f, 8.5f, 9.0f
		);

		fire(0,
			(PLAYFIELD_RIGHT - ORIGIN_DISTANCE_X_1), ORIGIN_Y_1, player_left
		);
	} else if(boss_phase_frame < 100) {
		spawnray.unput_update_render();
	} else if(boss_phase_frame == 100) {
		spawnray_init(
			(PLAYFIELD_LEFT + ORIGIN_DISTANCE_X_1),
			ORIGIN_Y_1,
			((PLAYFIELD_RIGHT - 1) - target_first)
		);
		fire(1, origin_x, origin_y, ((PLAYFIELD_RIGHT - 1) - target_first));
	} else if(boss_phase_frame < 150) {
		spawnray.unput_update_render();
	} else if(boss_phase_frame == 150) {
		spawnray_init(
			(PLAYFIELD_RIGHT - playfield_fraction_x(4 / 80.0f)),
			ORIGIN_Y_2,
			player_left
		);
		fire(2, origin_x, origin_y, player_left);
	} else if(boss_phase_frame < 200) {
		spawnray.unput_update_render();
	} else if(boss_phase_frame == 200) {
		spawnray_init(
			(PLAYFIELD_LEFT + playfield_fraction_x(3 / 80.0f)),
			ORIGIN_Y_2,
			player_left
		);
		fire(3, origin_x, origin_y, player_left);
	} else if(boss_phase_frame < 250) {
		spawnray.unput_update_render();
	} else if(boss_phase_frame > 300) {
		boss_phase_frame = 0;
	}

	#undef fire
	#undef spawnray_init
}
