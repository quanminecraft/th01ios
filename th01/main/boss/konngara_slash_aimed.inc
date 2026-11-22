inline void slash_aimed_fire(
	const screen_x_t& left, const screen_y_t& top, const subpixel_t& speed
) {
	Pellets.add_group(left, top, PG_1_AIMED, speed);
}

void pattern_slash_aimed(void)
{
	static screen_x_t spawner_left;
	static screen_y_t spawner_top;

	if(boss_phase_frame == 10) {
		face_direction_set_and_put(FD_CENTER);
		face_expression_set_and_put(FE_AIM);
		face_direction_can_change = false;
		spawner_left = SWORD_CENTER_X;
		spawner_top = SWORD_CENTER_Y;
		select_subpixel_for_rank(pattern_state.speed, 4.0f, 5.0f, 5.5f, 6.0f);
	}

	slash_animate();
	if(boss_phase_frame < SLASH_2_FRAME) {
		return;
	}
	if((boss_phase_frame < SLASH_4_FRAME) && ((boss_phase_frame % 3) == 0)) {
		slash_aimed_fire(spawner_left, spawner_top, pattern_state.speed);
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
		slash_aimed_fire(spawner_left, spawner_top, pattern_state.speed);
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
		slash_aimed_fire(spawner_left, spawner_top, pattern_state.speed);
		slash_spawner_step_from_4_5_to_6(spawner_left, spawner_top, 2);
	}
}

