void near pattern_aimed_sling_clusters(void)
{
	if(boss_phase_frame == 10) {
		select_for_rank(pattern_state.interval, 4, 3, 2, 2);
	}
	if(
		((boss_phase_frame % 200) >= 150) &&
		((boss_phase_frame % pattern_state.interval) == 0)
	) {
		screen_x_t left;
		vram_y_t top;
		unsigned char angle = irand();

		left = polar_y(PLAYFIELD_CENTER_X, (PLAYFIELD_W / 16), angle);
		top = polar_x(
			(SEAL_CENTER_Y - (PELLET_H / 2) - 1),
			playfield_fraction_y(5 / 42.0f),
			angle
		);
		Pellets.add_single(left, top, 0x00, 0, PM_SLING_AIMED, to_sp(4.5f));
	}
	if(boss_phase_frame >= 299) {
		boss_phase_frame = 0;
	}
}
