if(
			(u2.unused == 0) &&
			((boss_phase_frame % pattern_interval) == 0) &&
			!pentagram_in_slam_phase(pentagram_.phase)
		) {
			for(int i = 0; i < PENTAGRAM_POINTS; i++) {
				angle.tmp = iatan2(
					(pentagram.y[i] - pentagram.center.y),
					(pentagram.x[i] - pentagram.center.x)
				);
				Pellets.add_single(
					pentagram.x[i], pentagram.y[i], angle.tmp, to_sp(3.0f)
				);
			}
		}

		pentagram_prepare_update(pentagram_, PAP_PREPARE_1);
		pentagram_prepare_update(pentagram_, PAP_PREPARE_2);
		if(pentagram_in_clockwise_spin_phase(pentagram_.phase)) {
			if(pentagram_spin_unput_update_render(pentagram_, CLOCKWISE)) {
				#ifdef TH01_NATIVE_ORB_PHYSICS
                pentagram_.phase = static_cast<pentagram_attack_phase_t>(pentagram_.phase + 1);
#else
                reinterpret_cast<int &>(pentagram_.phase)++;
#endif
				boss_phase_frame = 0;
			}
		}
		if(pentagram_in_counterclockwise_spin_phase(pentagram_.phase)) {
			if(pentagram_spin_unput_update_render(
				pentagram_, COUNTERCLOCKWISE
			)) {
				#ifdef TH01_NATIVE_ORB_PHYSICS
                pentagram_.phase = static_cast<pentagram_attack_phase_t>(pentagram_.phase + 1);
#else
                reinterpret_cast<int &>(pentagram_.phase)++;
#endif
				boss_phase_frame = 0;
				pentagram.aim(player_left, player_bottom());
			}
		} else if(pentagram_.phase == PAP_SLAM_INTO_PLAYER_1) {
			if(pentagram_slam_unput_update_render(
				pentagram_, (PLAYER_H + 4 + PENTAGRAM_RADIUS_FINAL), false
			)) {
				phase.back_from_13_to_10(PAP_PREPARE_2);
			}
		} else if(pentagram_.phase == PAP_GROW) {
			if((boss_phase_frame % PENTAGRAM_INTERVAL) == 0) {
				pentagram.radius++;
				pentagram_.spin(CLOCKWISE);
				pentagram_.unput_update_render_regular();
			}
			if(boss_phase_frame >= SUBPHASE_TIMEOUT_FRAMES) {
				pentagram_.phase = PAP_SLAM_INTO_PLAYER_2;
				boss_phase_frame = 0;
				pentagram.aim(player_left, player_bottom());
			}
		}
		if(pentagram_.phase == PAP_SLAM_INTO_PLAYER_2) {
			if(pentagram_slam_unput_update_render(
				pentagram_, (GROWN_RADIUS + (PLAYER_H - 6)), true
			)) {
				phase.back_from_13_to_10(PAP_PREPARE_1);
			}
		}
		