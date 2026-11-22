void pattern_semicircle_rain_from_sleeve(void)
{
	enum {
		SPREAD = 10,
	};
	int i;
	unsigned char angle;

	if(boss_phase_frame == 10) {
		face_expression_set_and_put(FE_AIM);
	}
	if(boss_phase_frame < 100) {
		return;
	}
	if((boss_phase_frame % 20) == 0) {
		for(i = 0, angle = 0x00; i < SPREAD; i++, angle -= (0x80 / SPREAD)) {
			pellets_add_single_rain(
				LEFT_SLEEVE_LEFT, LEFT_SLEEVE_TOP, angle, 2.0f
			);
		}
		mdrv2_se_play(7);
	} else if((boss_phase_frame % 20) == 10) {
		for(i = 0, angle = 0x00; i < SPREAD; i++, angle -= (0x80 / SPREAD)) {
			Pellets.add_single(
				LEFT_SLEEVE_LEFT, LEFT_SLEEVE_TOP, angle, to_sp(2.0f),
				PM_FALL_STRAIGHT_FROM_TOP_THEN_REGULAR, to_sp(3.0f)
			);
		}
		mdrv2_se_play(7);
	}
	if(boss_phase_frame >= 300) {
		boss_phase_frame = 0;
	}
}

