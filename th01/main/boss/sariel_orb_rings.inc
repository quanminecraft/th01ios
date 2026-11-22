void near pattern_2_rings_from_a2_orbs(void)
{
	static unsigned char angle;
	static int interval;

	if(boss_phase_frame < 5) {
		angle = 0x00;
		select_for_rank(interval, 18, 12, 10, 8);
	}
	if((boss_phase_frame % interval) == 0) {
		Pellets.spawn_with_cloud = true;
		Pellets.add_single(
			A2_ORBS_R_CENTER_X, A2_ORBS_CENTER_Y, (0x00 - angle), to_sp(3.5f)
		);
		Pellets.add_single(
			A2_ORBS_R_CENTER_X, A2_ORBS_CENTER_Y, (0x80 - angle), to_sp(3.5f)
		);
		Pellets.add_single(
			A2_ORBS_L_CENTER_X, A2_ORBS_CENTER_Y, (0x00 + angle), to_sp(3.5f)
		);
		Pellets.add_single(
			A2_ORBS_L_CENTER_X, A2_ORBS_CENTER_Y, (0x80 + angle), to_sp(3.5f)
		);
		angle += interval;
	}
	if(boss_phase_frame > 479) {
		boss_phase_frame = 0;
	}
}
