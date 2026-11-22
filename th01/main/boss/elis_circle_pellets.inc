int pattern_pellets_along_circle(void)
{
	static StarCircle circle;
	screen_x_t left;
	screen_y_t top;

	if(boss_phase_frame < 10) {
		circle.frame = 0;
	}
	if(boss_phase_frame == 50) {
		ent_unput_and_put_both(ent_still_or_wave, 1, C_HAND, false);
		circle.angle = 0x00;
		#ifdef TH01_NATIVE_ORB_PHYSICS
        select_for_rank(pattern_state.group,
#else
select_for_rank(reinterpret_cast<int &>(pattern_state.group),
#endif
			PG_1_AIMED,
			PG_1_RANDOM_NARROW_AIMED,
			PG_3_SPREAD_WIDE_AIMED,
			// This will spawn (32 * 5) = 160 pellets, which is 60 more than
			// the original PELLET_COUNT. As a result, Elis will only ever fire
			// up to 20 of these spreads on Lunatic, rather than the coded 32.
			PG_5_SPREAD_NARROW_AIMED
		);
	}

	// ZUN quirk: Adding a `double` value < 1.0 to an integer is still a no-op.
	// That leads to the start and end angle for quadrant IV being identical,
	// and nothing being drawn there as a result. Hard to call it a bug though,
	// since this is the only function where this happens. But if it this was
	// intended after all, why not just remove the call for quadrant IV?!
	if(bigcircle_summon_and_flash(circle, 60, 0.05)) {
		bigcircle_sloppy_unput(circle);
		circle.angle = 0x00;
		for(int i = 0; i < 32; i++) {
			// lefteye?!
			left = polar_x(girl_lefteye_x(), BIGCIRCLE_RADIUS, circle.angle);

			top = polar_y(
				form_center_y(F_GIRL), BIGCIRCLE_RADIUS, circle.angle
			);
			Pellets.add_group(left, top, pattern_state.group, to_sp(0.25f));
			circle.angle += (0x100 / 32);
		}
		boss_phase_frame = 0;
		circle.frame = 0;
		return CHOOSE_NEW;
	}
	return 3;
}

