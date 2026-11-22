void pattern_bat_slow_spreads(void)
{
	if((boss_phase_frame % 16) == 0) {
		pellet_group_t group;
		#ifdef TH01_NATIVE_ORB_PHYSICS
        int group_id;
        select_for_rank(group_id,
#else
select_for_rank(reinterpret_cast<int &>(group),
#endif
			PG_3_SPREAD_WIDE_AIMED,
			PG_5_SPREAD_WIDE_AIMED,
			PG_5_SPREAD_NARROW_AIMED,

			// Not aimed on Lunatic? Probably the easiest version then, since
			// pellets are always fired down in this case.
			// (Compare pattern_souls_spreads() in the Kikuri fight.)
			PG_5_SPREAD_NARROW
		);
		#ifdef TH01_NATIVE_ORB_PHYSICS
        group=static_cast<pellet_group_t>(group_id);
#endif
form_fire_group(F_BAT, group, 3.5f);
	}
}

void pattern_bat_alternating_narrow_and_wide_2_spreads(void)
{
	if((boss_phase_frame % 4) == 0) {
		if(irand() & 1) {
			form_fire_group(F_BAT, PG_2_SPREAD_NARROW_AIMED, 5.5f);
		} else {
			form_fire_group(F_BAT, PG_2_SPREAD_WIDE_AIMED, 5.5f);
		}
	}
}

void pattern_bat_alternating_2_and_3_spreads(void)
{
	if(((boss_phase_frame % 50) < 10) && ((boss_phase_frame % 4) == 0)) {
		form_fire_group(F_BAT, PG_2_SPREAD_NARROW_AIMED, 5.3125f);
	}
	if(
		((boss_phase_frame % 50) >= 25) &&
		((boss_phase_frame % 50) < 35) &&
		((boss_phase_frame % 4) == 0)
	) {
		form_fire_group(F_BAT, PG_3_SPREAD_NARROW_AIMED, 5.3125f);
	}
}

void pattern_bat_random_rain(void)
{
	if((boss_phase_frame % 4) == 0) {
		pellets_add_single_rain(
			(form_center_x(F_BAT) - (PELLET_W / 2)),
			(form_center_y(F_BAT) - (PELLET_H / 2)),
			irand(),
			0.125f
		);
	}
}

