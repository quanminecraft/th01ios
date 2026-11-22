void near pattern_rain_from_top(void)
{
	if((boss_phase_frame % 8) != 0) {
		return;
	}
	screen_x_t left = playfield_rand_x();
	vram_y_t top = PLAYFIELD_TOP;
	pellet_group_t group;

#ifdef TH01_NATIVE_ORB_PHYSICS
	int selected;
	select_for_rank(selected, PG_1, PG_1, PG_1, PG_1_RANDOM_WIDE);
	group = static_cast<pellet_group_t>(selected);
#else
	select_for_rank(
		reinterpret_cast<int &>(group), PG_1, PG_1, PG_1, PG_1_RANDOM_WIDE
	);
#endif
	Pellets.add_group(left, top, group, to_sp(2.75f));
}
