void pattern_diamond_cross_to_edges_followed_by_rain(void)
{
	enum {
		DIAMOND_ORIGIN_X = (PLAYFIELD_CENTER_X - (DIAMOND_W / 2)),
		DIAMOND_ORIGIN_Y = (PLAYFIELD_CENTER_Y + (DIAMOND_H / 2)),
	};

	int i;

	static pixel_t velocity_bottomleft_x, velocity_topleft_x;
	static pixel_t velocity_bottomleft_y, velocity_topleft_y;
	static EntitiesTopleft<4> diamonds;
	static int diamonds_at_edges_frame;

	#define diamonds_unput(i) \
		for(i = 0; i < diamonds.count(); i++) { \
			shape8x8_sloppy_unput(diamonds.left[i], diamonds.top[i]); \
		}

	#define diamonds_put(i) \
		for(i = 0; i < diamonds.count(); i++) { \
			shape8x8_diamond_put(diamonds.left[i], diamonds.top[i], 9); \
		}

	if(boss_phase_frame == 10) {
		face_expression_set_and_put(FE_NEUTRAL);
	}
	if(boss_phase_frame < 100) {
		return;
	} else if(boss_phase_frame == 100) {
		// MODDERS: Just use a local variable.
		select_for_rank(pattern_state.group,
			PG_2_SPREAD_NARROW_AIMED,
			PG_3_SPREAD_NARROW_AIMED,
			PG_5_SPREAD_WIDE_AIMED,
			PG_5_SPREAD_NARROW_AIMED
		);

		vector2_between(
			DIAMOND_ORIGIN_X, DIAMOND_ORIGIN_Y,
			PLAYFIELD_LEFT, player_center_y(),
			velocity_bottomleft_x, velocity_bottomleft_y,
			7
		);
		vector2_between(
			DIAMOND_ORIGIN_X, DIAMOND_ORIGIN_Y,
			PLAYFIELD_LEFT, PLAYFIELD_TOP,
			velocity_topleft_x, velocity_topleft_y,
			7
		);

		for(i = 0; i < diamonds.count(); i++) {
			diamonds.left[i] = DIAMOND_ORIGIN_X;
			diamonds.top[i] = DIAMOND_ORIGIN_Y;
		}
		Pellets.add_group(
			(PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - PELLET_W),
			(PLAYFIELD_TOP + playfield_fraction_y(8 / 21.0f) - (PELLET_H / 2)),
			static_cast<pellet_group_t>(pattern_state.group),
			to_sp(3.0f)
		);
		select_for_rank(pattern_state.interval, 18, 16, 14, 12);
		mdrv2_se_play(12);
	} else if(diamonds.left[0] > PLAYFIELD_LEFT) {
		diamonds_unput(i);
		diamonds.left[0] += velocity_bottomleft_x;
		diamonds.top[0]  += velocity_bottomleft_y;
		diamonds.left[1] -= velocity_bottomleft_x;
		diamonds.top[1]  += velocity_bottomleft_y;
		diamonds.left[2] += velocity_topleft_x;
		diamonds.top[2]  += velocity_topleft_y;
		diamonds.left[3] -= velocity_topleft_x;
		diamonds.top[3]  += velocity_topleft_y;
		if(diamonds.left[0] <= PLAYFIELD_LEFT) {
			diamonds.left[0] = PLAYFIELD_LEFT;
			diamonds.left[2] = PLAYFIELD_LEFT;
			diamonds.left[1] = (PLAYFIELD_RIGHT - DIAMOND_W);
			diamonds.left[3] = (PLAYFIELD_RIGHT - DIAMOND_W);
		} else {
			diamonds_put(i);
		}
		return;
	} else if(diamonds.top[0] > PLAYFIELD_TOP) {
		diamonds_unput(i);
		diamonds.top[0] -= 3;
		diamonds.top[1] -= 3;
		diamonds.left[2] += 6;
		diamonds.left[3] -= 6;
		if(diamonds.top[0] <= PLAYFIELD_TOP) {
			diamonds.top[0] = PLAYFIELD_TOP;
		} else {
			diamonds_put(i);
		}
		return;
	} else if(diamonds_at_edges_frame < 200) {
		diamonds_at_edges_frame++;
		if((diamonds_at_edges_frame % pattern_state.interval) == 0)  {
			#define speed to_sp(2.5f)
			screen_x_t from_left;
			screen_y_t from_top;
			screen_x_t to_left;
			screen_y_t to_top;
			unsigned char angle;

			from_left = PLAYFIELD_LEFT;
			from_top = playfield_rand_y(0.0f, (25 / 42.0f));
			to_left = playfield_rand_x(0.375f, 1.0f);
			to_top = PLAYFIELD_BOTTOM;
			angle = iatan2((to_top - from_top), (to_left - from_left));
			Pellets.add_single(from_left, from_top, angle, speed);

			from_left = (PLAYFIELD_RIGHT - PELLET_W);
			from_top = playfield_rand_y(0.0f, (25 / 42.0f));
			to_left  = playfield_rand_x(0.0f, 0.625f);
			to_top = PLAYFIELD_BOTTOM;
			angle = iatan2((to_top - from_top), (to_left - from_left));
			Pellets.add_single(from_left, from_top, angle, speed);

			from_top = PLAYFIELD_TOP;
			from_left = playfield_rand_x();
			to_top = PLAYFIELD_BOTTOM;
			to_left = playfield_rand_x();
			angle = iatan2((to_top - from_top), (to_left - from_left));
			Pellets.add_single(from_left, from_top, angle, speed);

			from_top = PLAYFIELD_TOP;
			from_left = playfield_rand_x();
			to_top = PLAYFIELD_BOTTOM;
			to_left = playfield_rand_x();
			angle = iatan2((to_top - from_top), (to_left - from_left));
			Pellets.add_single(from_left, from_top, angle, speed);

			from_top = PLAYFIELD_TOP;
			from_left = playfield_rand_x();
			Pellets.add_group(from_left, from_top, PG_1_AIMED, speed);

			#undef speed
		}
		return;
	} else {
		boss_phase_frame = 0;
	}
	diamonds_at_edges_frame = 0;

	#undef diamonds_put
	#undef diamonds_unput
}

void pattern_symmetrical_from_cup_fire(unsigned char angle)
{
	Pellets.add_single(CUP_RIGHT, CUP_TOP, angle, pattern_state.speed);
	Pellets.add_single(CUP_LEFT,  CUP_TOP, angle, pattern_state.speed);
}

void pattern_symmetrical_from_cup(void)
{
	static unsigned char angle;

	if(boss_phase_frame == 10) {
		face_expression_set_and_put(FE_CLOSED);
	}
	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		angle = 0x40;
		select_subpixel_for_rank(pattern_state.speed, 5.0f, 5.0f, 6.0f, 7.0f);
	}
	if((boss_phase_frame < 140) && ((boss_phase_frame % 8) == 0)) {
		pattern_symmetrical_from_cup_fire(0x40);
		return;
	}
	if((boss_phase_frame < 220) && ((boss_phase_frame % 8) == 0)) {
		angle += 0x05;
		pattern_symmetrical_from_cup_fire(angle);
		pattern_symmetrical_from_cup_fire(0x80 - angle);
		return;
	}
	if((boss_phase_frame < 300) && ((boss_phase_frame % 8) == 0)) {
		angle -= 0x0C;
		pattern_symmetrical_from_cup_fire(angle);
		pattern_symmetrical_from_cup_fire(0x80 - angle);
	}
	if(boss_phase_frame >= 300) {
		boss_phase_frame = 0;
	}
}

