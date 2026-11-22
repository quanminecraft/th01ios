laser_eye.locked_unput_and_put_8();

		if((boss_phase_frame >= 30) && (
			(u2.subphase == laser_subphase(0, LSP_PREPARE)) ||
			(u2.subphase == laser_subphase(1, LSP_PREPARE)) ||
			(u2.subphase == laser_subphase(2, LSP_PREPARE))
		)) {
			laser_eye.set_image(C_CLOSED);
		}
		if(laser_pattern_should_run(u2.subphase, 0)) {
			pattern_single_laser_across_playfield(
				u2.subphase, pentagram_.angle, u1.x_edge_offset, 0, X_RIGHT
			);
		} else if(laser_pattern_should_run(u2.subphase, 1)) {
			pattern_single_laser_across_playfield(
				u2.subphase, pentagram_.angle, u1.x_edge_offset, 1, X_LEFT
			);
		} else if(laser_pattern_should_run(u2.subphase, 2)) {
			pattern_dual_lasers_across_playfield(
				u2.subphase, pentagram_.angle, u1.x_edge_offset, 2
			);
		}
		