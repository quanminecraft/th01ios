eyes_locked_unput_and_put_then_track(
			EF_WEST | EF_EAST | EF_SOUTHWEST | EF_SOUTHEAST | EF_NORTH
		);
		if(
			(boss_phase_frame == SUBPHASE_PREPARE_FRAMES) ||
			(u2.subphase == P11_CORNERS_IN_EYES)
		) {
			if(u2.subphase == P11_PREPARE) {
				u2.subphase = P11_CORNERS_IN_EYES;
				target_left = player_left; // unused
			}

			// Every frame, for a change? No unblitting glitches here, then. :P
			pentagram_between_eyes_put();

			if(boss_phase_frame == 140) {
				u2.subphase = P11_MOVE;
				u1.distance = 0;
			}
		}

		if(
			(u2.subphase == P11_MOVE) &&
			((boss_phase_frame % PENTAGRAM_INTERVAL) == 1)
		) {
			// ZUN bug: Doing this after blitting the eyes is guaranteed to
			// rip holes into them for at least one frame.
			pentagram_shrink(pentagram_shrink_unput, u1.distance);

			u1.distance += (PENTAGRAM_SHRINK_DISTANCE / 20);
			pentagram_shrink(pentagram_shrink_put, u1.distance);

			// ZUN quirk: Should be >=, and ideally even be clamped to exactly
			// that value. With >, the shrink animation lasts for one step
			// longer than it should, leaving the pentagram both smaller and
			// moved slightly off-center.
			if(u1.distance > PENTAGRAM_SHRINK_DISTANCE) {
				// ZUN quirk: This uses offcenter_x(), which makes sure that
				// the regular pentagram will stay off-center even *if* the
				// above condition were corrected.
				pentagram.x[0] = pentagram_shrink_x(EF_NORTH, u1.distance);
				pentagram.y[0] = pentagram_shrink_y(EF_NORTH, u1.distance);

				u2.subphase = P11_DONE;
			}
		}
		