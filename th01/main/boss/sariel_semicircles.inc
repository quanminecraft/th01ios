void near pattern_four_semicircle_spreads(void)
{
	enum {
		ORIGIN_X = SHIELD_CENTER_X,
		ORIGIN_Y = (SHIELD_CENTER_Y - (PELLET_H / 2)),
	};

	int i;
	unsigned char angle;

	if(boss_phase_frame < 50) {
		return;
	} else if(boss_phase_frame == 50) {
		for((angle = 0x00, i = 0); i < 20; (angle += 0x07, i++)) {
			Pellets.add_single(ORIGIN_X, ORIGIN_Y, angle, to_sp(2.0f));
		}
	} else if(boss_phase_frame == 60) {
		for((angle = 0x04, i = 0); i < 10; (angle += 0x0C, i++)) {
			Pellets.add_single(ORIGIN_X, ORIGIN_Y, angle, to_sp(3.0f));
		}
	} else if(boss_phase_frame == 70) {
		for((angle = 0x04, i = 0); i < 8; (angle += 0x0E, i++)) {
			Pellets.add_single(ORIGIN_X, ORIGIN_Y, angle, to_sp(4.0f));
		}
	} else if(boss_phase_frame == 80) {
		for((angle = 0x0E, i = 0); i < 10; (angle += 0x0A, i++)) {
			Pellets.add_single(ORIGIN_X, ORIGIN_Y, angle, to_sp(3.0f));
		}
	} else if(boss_phase_frame > 150) {
		boss_phase_frame = 0;
	}
}
