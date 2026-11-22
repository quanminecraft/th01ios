enum slash_cel_frame_t {
	SLASH_0_FRAME = 50,
	SLASH_1_FRAME = 60,
	SLASH_2_FRAME = 100,
	SLASH_3_FRAME = 120,
	SLASH_4_FRAME = 140,
	SLASH_4_5_FRAME = 150,
	SLASH_5_FRAME = 160,
	SLASH_6_FRAME = 170,

	SLASH_FRAMES_FROM_2_TO_4 = (SLASH_4_FRAME - SLASH_2_FRAME),
	 // Yes, also backwards
	SLASH_FRAMES_FROM_5_TO_4_5 = (SLASH_4_5_FRAME - SLASH_5_FRAME),
	SLASH_FRAMES_FROM_4_5_TO_6 = (SLASH_6_FRAME - SLASH_4_5_FRAME),
};

void slash_animate(void)
{
	// MODDERS: Just use a switch.
	#define boss_phase_at_frame(frame) \
		boss_phase_frame < frame) return; if(boss_phase_frame == frame

	if(boss_phase_at_frame(SLASH_0_FRAME)) {
		slash_put(0);
		mdrv2_se_play(8);
	}
	if(boss_phase_at_frame(SLASH_1_FRAME)) {
		slash_put(1);
	}
	if(boss_phase_at_frame(SLASH_2_FRAME)) {
		slash_put(2);
	}
	if(boss_phase_at_frame(SLASH_3_FRAME)) {
		slash_put(3);
	}
	if(boss_phase_at_frame(SLASH_4_FRAME)) {
		slash_put(4);
	}
	if(boss_phase_at_frame(SLASH_5_FRAME)) {
		slash_put(5);
	}
	if(boss_phase_at_frame(SLASH_6_FRAME)) {
		slash_put(6);
	}
	if(boss_phase_frame > 200) {
		boss_phase_frame = 0;
		face_direction_can_change = true;
	}

	#undef boss_phase_at_frame
}

#define slash_spawner_step_from_2_to_4(left, top, steps) \
	left += (SLASH_DISTANCE_2_TO_4_X / (SLASH_FRAMES_FROM_2_TO_4 / steps)); \
	top  += (SLASH_DISTANCE_2_TO_4_Y / (SLASH_FRAMES_FROM_2_TO_4 / steps));

#define slash_spawner_step_from_4_to_4_5(left, top, steps) \
	left -= (SLASH_DISTANCE_5_TO_4_X / (SLASH_FRAMES_FROM_5_TO_4_5 / steps)); \
	top  -= (SLASH_DISTANCE_5_TO_4_Y / (SLASH_FRAMES_FROM_5_TO_4_5 / steps));

#define slash_spawner_step_from_4_5_to_6(left, top, steps) \
	left -= (SLASH_DISTANCE_5_TO_6_X / (SLASH_FRAMES_FROM_4_5_TO_6 / steps)); \
	top  -= (SLASH_DISTANCE_5_TO_6_Y / (SLASH_FRAMES_FROM_4_5_TO_6 / steps));

inline void slash_rain_fire(const screen_x_t& left, const screen_y_t& top) {
	pellets_add_single_rain(left, top, (irand() % 0x7F), 0.0f);
	pellets_add_single_rain(left, top, (irand() % 0x7F), 0.0f);
}

void pattern_slash_rain(void)
{
	static screen_x_t spawner_left;
	static screen_y_t spawner_top;

	if(boss_phase_frame == 10) {
		face_direction_set_and_put(FD_CENTER);
		face_expression_set_and_put(FE_CLOSED);
		face_direction_can_change = false;
		spawner_left = SWORD_CENTER_X;
		spawner_top = SWORD_CENTER_Y;
		select_for_rank(pattern_state.interval, 5, 3, 2, 1);
	}

	slash_animate();

	if(boss_phase_frame < SLASH_2_FRAME) {
		return;
	}
	if(
		(boss_phase_frame < SLASH_4_FRAME) &&
		((boss_phase_frame % pattern_state.interval) == 0)
	) {
		slash_rain_fire(spawner_left, spawner_top);
		slash_spawner_step_from_2_to_4(
			spawner_left, spawner_top, pattern_state.interval
		);
	}

	if(boss_phase_frame == SLASH_4_FRAME) {
		spawner_left = SLASH_4_CORNER_X;
		spawner_top = SLASH_4_CORNER_Y;
		// Originally meant to be the step interval between cels 4 and 5?
		select_for_rank(pattern_state.unused, 3, 2, 2, 2);
	}
	if(boss_phase_frame < SLASH_4_FRAME) {
		return;
	}
	if(boss_phase_frame < SLASH_4_5_FRAME) {
		slash_rain_fire(spawner_left, spawner_top);
		slash_spawner_step_from_4_to_4_5(spawner_left, spawner_top, 1);
	}

	if(boss_phase_frame == SLASH_4_5_FRAME) {
		spawner_left = SLASH_5_CORNER_X;
		spawner_top = SLASH_5_CORNER_Y;
		select_for_rank(pattern_state.interval, 3, 2, 1, 1);
	}
	if(boss_phase_frame < SLASH_4_5_FRAME) {
		return;
	}
	if(
		(boss_phase_frame < SLASH_6_FRAME) &&
		((boss_phase_frame % pattern_state.interval) == 0)
	) {
		slash_rain_fire(spawner_left, spawner_top);
		slash_spawner_step_from_4_5_to_6(
			spawner_left, spawner_top, pattern_state.interval
		);
	}
}

inline void slash_triangular_fire(
	const screen_x_t& left, const screen_y_t& top, const subpixel_t& speed
) {
	Pellets.add_single(left, top, 0x20, speed);
	Pellets.add_single(left, top, 0x60, speed);
}

void pattern_slash_triangular(void)
{
	static screen_x_t spawner_left;
	static screen_y_t spawner_top;

	if(boss_phase_frame == 10) {
		face_direction_set_and_put(FD_CENTER);
		face_expression_set_and_put(FE_AIM);
		face_direction_can_change = false;
		spawner_left = SWORD_CENTER_X;
		spawner_top = SWORD_CENTER_Y;
		select_subpixel_for_rank(pattern_state.speed, 2.0f, 3.0f, 4.0f, 4.5f);
	}

	slash_animate();

	if(boss_phase_frame < SLASH_2_FRAME) {
		return;
	}
	if((boss_phase_frame < SLASH_4_FRAME) && ((boss_phase_frame % 3) == 0)) {
		slash_triangular_fire(spawner_left, spawner_top, pattern_state.speed);
		slash_spawner_step_from_2_to_4(spawner_left, spawner_top, 3);
	}

	if(boss_phase_frame == SLASH_4_FRAME) {
		spawner_left = SLASH_4_CORNER_X;
		spawner_top = SLASH_4_CORNER_Y;
	}
	if(boss_phase_frame < SLASH_4_FRAME) {
		return;
	}
	if(boss_phase_frame < SLASH_4_5_FRAME) {
		slash_triangular_fire(spawner_left, spawner_top, pattern_state.speed);
		slash_spawner_step_from_4_to_4_5(spawner_left, spawner_top, 1);
	}

	if(boss_phase_frame == SLASH_4_5_FRAME) {
		spawner_left = SLASH_5_CORNER_X;
		spawner_top = SLASH_5_CORNER_Y;
	}
	if(boss_phase_frame < SLASH_4_5_FRAME) {
		return;
	}
	if((boss_phase_frame < SLASH_6_FRAME) && ((boss_phase_frame % 2) == 0)) {
		slash_triangular_fire(spawner_left, spawner_top, pattern_state.speed);
		slash_spawner_step_from_4_5_to_6(spawner_left, spawner_top, 2);
	}
}

