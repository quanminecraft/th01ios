elis_starpattern_ret_t pattern_three_symmetric_4_stacks_then_symmetric_arc(void)
{
	#define fire_symmetric(angle_right, speed) { \
		Pellets.add_single( \
			(form_center_x(F_GIRL) - (PELLET_W / 2) - 4), \
			(form_center_y(F_GIRL) - (PELLET_H / 2) - 4), \
			(0x80 - angle_right), \
			TO_SP(speed) \
		); \
		Pellets.add_single( \
			(form_center_x(F_GIRL) - (PELLET_W / 2) + 4), \
			(form_center_y(F_GIRL) - (PELLET_H / 2) - 4), \
			angle_right, \
			TO_SP(speed) \
		); \
	}

	ent_attack_render();
	if(boss_phase_frame == 50) {
		select_for_rank(pattern_state.speed, 2, 3, 3, 4);
		for(int i = 0; i < 4; i++) {
			fire_symmetric(0x40, (i + pattern_state.speed));
		}
	} else if(boss_phase_frame == 60) {
		for(int i = 0; i < 4; i++) {
			fire_symmetric(0x30, (i + pattern_state.speed));
		}
		mdrv2_se_play(7);
	} else if(boss_phase_frame == 70) {
		for(int i = 0; i < 4; i++) {
			fire_symmetric(0x18, (i + pattern_state.speed));
		}
		mdrv2_se_play(7);
	} else if(boss_phase_frame == 80) {
		unsigned char angle = 0x00;
		for(int i = 0; i < 10; i++) {
			fire_symmetric(angle, (2 + pattern_state.speed));
			angle += 0x06;
		}
		mdrv2_se_play(7);
	} else if(boss_phase_frame >= 120) {
		boss_phase_frame = 0;
		return SP_STAR_OF_DAVID;
	}
	return SP_PATTERN;

	#undef fire_symmetric
}

