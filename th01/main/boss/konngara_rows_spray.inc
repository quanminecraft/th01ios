void pattern_aimed_rows_from_top(void)
{
	enum {
		DIAMOND_SPEED = 8,
		ROW_MARGIN = (PLAYFIELD_W / 10),
	};

	static point_t diamond_velocity;

	// screen_point_t would generate too good ASM here
	static screen_x_t diamond_left;
	static screen_y_t diamond_top;

	static enum {
		RIGHT = 0,
		LEFT = 1,
		DOWN_START = 2,
		DOWN_END = (DOWN_START + (ROW_MARGIN / DIAMOND_SPEED)),
		TO_INITIAL_POSITION = 99,

		_diamond_direction_t_FORCE_INT16 = 0x7FFF
	} diamond_direction;
	static Subpixel pellet_speed;

	if(boss_phase_frame == 10) {
		face_expression_set_and_put(FE_NEUTRAL);
	}
	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		vector2_between(
			LEFT_SLEEVE_LEFT, LEFT_SLEEVE_TOP,
			(PLAYFIELD_LEFT + ROW_MARGIN), PLAYFIELD_TOP,
			diamond_velocity.x, diamond_velocity.y,
			(DIAMOND_SPEED * 2)
		);
		diamond_left = LEFT_SLEEVE_LEFT;
		diamond_top = LEFT_SLEEVE_TOP;
		pellet_speed.set(3.0f);
		mdrv2_se_play(12);
		diamond_direction = TO_INITIAL_POSITION;
		select_for_rank(pattern_state.interval, 12, 10, 8, 6);
		return;
	}
	if(diamond_direction == TO_INITIAL_POSITION) {
		shape8x8_sloppy_unput(diamond_left, diamond_top);
		diamond_left += diamond_velocity.x;
		diamond_top += diamond_velocity.y;

		if(diamond_top <= PLAYFIELD_TOP) {
			diamond_left = (PLAYFIELD_LEFT + ROW_MARGIN);
			diamond_top = PLAYFIELD_TOP;
			diamond_direction = RIGHT;
			return;
		}
		shape8x8_diamond_put(diamond_left, diamond_top, 9);
	} else if(diamond_direction < TO_INITIAL_POSITION) {
		shape8x8_sloppy_unput(diamond_left, diamond_top);

		// That's quite the roundabout way of saying "-8, 0, or +8"...
		diamond_left += (DIAMOND_SPEED + (
			(diamond_direction == RIGHT) ?   0 :
			(diamond_direction ==  LEFT) ? (-DIAMOND_SPEED * 2) :
			/*            >= DOWN_START */  -DIAMOND_SPEED
		));
		if(diamond_direction >= DOWN_START) {
			diamond_top += DIAMOND_SPEED;
			#ifdef TH01_NATIVE_ORB_PHYSICS
			diamond_direction = static_cast<decltype(diamond_direction)>(diamond_direction + 1);
#else
			reinterpret_cast<int &>(diamond_direction)++;
#endif
			if(diamond_direction >= DOWN_END) {
				diamond_direction = (diamond_left > PLAYFIELD_CENTER_X)
					? LEFT
					: RIGHT;
			}
		}
		if(diamond_left > (PLAYFIELD_RIGHT - ROW_MARGIN)) {
			diamond_direction = DOWN_START;
			diamond_left = (PLAYFIELD_RIGHT - ROW_MARGIN);
		} else if(diamond_left < (PLAYFIELD_LEFT + ROW_MARGIN)) {
			diamond_direction = DOWN_START;
			diamond_left = (PLAYFIELD_LEFT + ROW_MARGIN);
		}
		if(diamond_top >= (PLAYFIELD_TOP + (ROW_MARGIN * 3))) {
			boss_phase_frame = 0;
			return;
		}
		if(diamond_direction < DOWN_START) {
			if((boss_phase_frame % pattern_state.interval) == 0) {
				Pellets.add_group(
					diamond_left, diamond_top, PG_1_AIMED, pellet_speed
				);
				pellet_speed += 0.125f;
			}
		}
		shape8x8_diamond_put(diamond_left, diamond_top, 6);
	}
}

void pattern_aimed_spray_from_cup(void)
{
	static unsigned char spray_offset;
	static unsigned char angle; // should be local
	static int spray_delta; // should be unsigned char
	static int pellets_fired_in_current_direction;

	if(boss_phase_frame == 10) {
		face_expression_set_and_put(FE_CLOSED);
	}
	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		spray_offset = 0x20;
		spray_delta = -0x08;
		pellets_fired_in_current_direction = 0;
		select_for_rank(pattern_state.interval, 5, 4, 3, 2);
	}
	if((boss_phase_frame % pattern_state.interval) == 0) {
		// Yes, the point from which these are aimed to the top-left player
		// coordinate is quite a bit away from where they're actually fired,
		// leading to some quite imperfect aiming. Probably done on purpose
		// though, and largely mitigated by the spraying motion anyway.
		angle = iatan2(
			(player_top - (CUP_TOP - 4)), (player_left - (CUP_CENTER_X - 34))
		);
		angle += spray_offset;
		spray_offset += spray_delta;
		pellets_fired_in_current_direction++;
		if(pellets_fired_in_current_direction > 8) {
			spray_delta *= -1;
			pellets_fired_in_current_direction = 0;
		}
		Pellets.add_single(CUP_CENTER_X, CUP_TOP, angle, to_sp(3.0f));
	}
	if(boss_phase_frame >= 700) {
		boss_phase_frame = 0;
	}
}

