elis_starpattern_ret_t pattern_aimed_5_spreads_and_lasers_followed_by_ring(void)
{
	enum {
		KEYFRAME_INIT = 50,
		KEYFRAME_START = 60,
	};

	static StarCircle circle;
	screen_x_t left;
	screen_x_t top;

	ent_attack_render();

	if(boss_phase_frame < KEYFRAME_INIT) {
		return SP_PATTERN;
	} else if(boss_phase_frame == KEYFRAME_INIT) {
		ent_unput_and_put_both(ent_still_or_wave, 1, C_HAND, false);
		circle.angle = 0x00;
		select_for_rank(pattern_state.interval, 40, 30, 20, 15);
	}
	if(bigcircle_summon_and_flash(circle, KEYFRAME_START, 0x02)) {
		bigcircle_sloppy_unput(circle);
		circle.angle = 0x00;

		// Depending on Elis' position, most of these pellets might be clipped.
		for(int i = 0; i < 32; i++) {
			left = polar_x(
				form_center_x(F_GIRL), BIGCIRCLE_RADIUS, circle.angle
			);

			// lefteye?!
			top = polar_y(girl_lefteye_y(), BIGCIRCLE_RADIUS, circle.angle);

			Pellets.add_group(left, top, PG_1, to_sp(4.0f));
			circle.angle += (0x100 / 32);
		}
		boss_phase_frame = 0;
		circle.frame = 0;
		return SP_STAR_OF_DAVID;
	}
	if((boss_phase_frame % pattern_state.interval) == 0) {
		Pellets.add_group(
			(girl_wing_left_center_x() - (PELLET_W / 2)),
			(girl_wing_center_y() - (PELLET_H / 2)),
			PG_5_SPREAD_WIDE_AIMED,
			to_sp(4.0f)
		);
		Pellets.add_group(
			(girl_wing_right_center_x() - (PELLET_W / 2)),
			(girl_wing_center_y() - (PELLET_H / 2)),
			PG_5_SPREAD_WIDE_AIMED,
			to_sp(4.0f)
		);
	}
	if((boss_phase_frame % (pattern_state.interval * 2)) == 0) {
		shootout_laser_safe(boss_phase_frame / 10).spawn(
			(form_center_x(F_GIRL) - (8 / 2)),
			girl_lefteye_y(),
			player_left,
			player_bottom(),
			(to_sp(6.25f) / 2),
			V_WHITE,
			20,
			8
		);
		mdrv2_se_play(6);
	}
	return SP_PATTERN;
}

