int pattern_random_downwards_missiles(void)
{
	static EntitiesTopleft<5> rifts;
	int cel; // ACTUAL TYPE: elis_grc_cel_t
	pixel_t velocity_x;
	pixel_t velocity_y;
	unsigned char angle;

	if(boss_phase_frame == 50) {
		ent_unput_and_put_both(ent_still_or_wave, 1, C_HAND, false);
		select_for_rank(pattern_state.angle_range, 0x0F, 0x15, 0x19, 0x1D);
	}

	// That's quite the brave placement for this branch...
	if((boss_phase_frame > 60) && ((boss_phase_frame % 3) == 0)) {
		int i = (irand() % rifts.count());
		angle = (
			(irand() % pattern_state.angle_range) -
			((pattern_state.angle_range - 0x01) / 2) +
			0x40
		);
		vector2(velocity_x, velocity_y, 7, angle);
		Missiles.add(rifts.left[i], rifts.top[i], velocity_x, velocity_y);
	}

	rifts_update_and_render(rifts, ent_still_or_wave, 60, 160, COL_FX, cel);

	if(boss_phase_frame > 170) {
		boss_phase_frame = 0;
		return CHOOSE_NEW;
	}
	return 2;
}

