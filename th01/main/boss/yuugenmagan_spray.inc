// ZUN bloat: Redundant and confusing. Just use the regular
		// [boss_phase_frame] exclusively?
		u1.subphase_frame++;

		angle.pellet_east--;

		eyes_locked_unput_and_put_then_track(EF_WEST | EF_EAST);

		if(
			(boss_phase_frame == SUBPHASE_PREPARE_FRAMES) &&
			(u2.subphase == P5_PREPARE_1)
		) {
			u1.subphase_frame = 0;
			u2.subphase = P5_SPRAY_1;
		}
		if(u2.subphase == P5_SPRAY_1) {
			if((u1.subphase_frame % pattern_interval) == 0) {
				eye_west.fire_from_bottom_center(
					true, angle.pellet_east, 4.0f
				);
				eye_east.fire_from_bottom_center(
					false, angle.pellet_east, 4.0f
				);
			}
			if((u1.subphase_frame / 5) >= 36) {
				u2.subphase = P5_PREPARE_2;
				boss_phase_frame = 0;
			}
		}
		if(
			(u2.subphase == P5_PREPARE_2) &&
			(boss_phase_frame > (SUBPHASE_PREPARE_FRAMES / 2))
		) {
			u1.subphase_frame = -15;
			u2.subphase = P5_SPRAY_2;
			angle.pellet_east = 0x80;
		}
		if(u2.subphase == P5_SPRAY_2) {
			if((u1.subphase_frame % pattern_interval) == 0) {
				// Since these pellets don't reach the top of the playfield
				// before this subphase ends, they don't actually move
				// differently compared to the first subphase.
				eye_west.fire_from_bottom_center(
					true,
					angle.pellet_east,
					4.0f,
					PM_FALL_STRAIGHT_FROM_TOP_THEN_REGULAR,
					4.0f
				);
				eye_east.fire_from_bottom_center(
					false,
					angle.pellet_east,
					4.0f,
					PM_FALL_STRAIGHT_FROM_TOP_THEN_REGULAR,
					4.0f
				);
			}
			if((u1.subphase_frame / 5) >= 20) {
				u2.subphase = P5_PREPARE_1;
				boss_phase_frame = 0;
				u3.iterations_done++;
				angle.pellet_east = 0x00;
			}
		}
		