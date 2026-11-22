inline void fire_from_lateral(pellet_group_t group, float speed_base) {
	eye_west.fire_from_bottom_center(group, speed_base);
	eye_east.fire_from_bottom_center(group, speed_base);
}

void phase_1_pellets_from_lateral()
{
	if((boss_phase_frame % pattern_interval) == 10) {
		fire_from_lateral(PG_2_SPREAD_WIDE_AIMED, 1.5f);
	} else if((boss_phase_frame % pattern_interval) == 25) {
		fire_from_lateral(PG_2_SPREAD_WIDE_AIMED, 1.75f);
	} else if((boss_phase_frame % pattern_interval) == 40) {
		fire_from_lateral(PG_2_SPREAD_WIDE_AIMED, 2.0f);
	} else if((boss_phase_frame % pattern_interval) == 60) {
		fire_from_lateral(PG_2_SPREAD_WIDE_AIMED, 2.75f);
	} else if((boss_phase_frame % pattern_interval) == 80) {
		fire_from_lateral(PG_2_SPREAD_WIDE_AIMED, 3.125f);
	} else if((boss_phase_frame % pattern_interval) == 110) {
		fire_from_lateral(PG_1_AIMED, 4.25f);
	}
}
// ------

