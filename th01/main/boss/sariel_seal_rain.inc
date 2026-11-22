void near pattern_rain_from_seal_center(sariel_frame_t &frame)
{
	enum {
		VELOCITY_Y = 8,
	};

	static struct SymmetricSpawnraysWithDebris<
		SEAL_CENTER_X, SEAL_CENTER_Y, DEBRIS_W
	> rays;
	static vortex_or_debris_cel_t debris_cel_cur;
	static vortex_or_debris_cel_t debris_cel_prev;

	if(frame < 50) {
		return;
	} else if(frame == 50) {
		rays.target_l_x = PLAYFIELD_LEFT;
		rays.target_y = (PLAYFIELD_BOTTOM - 1);
		debris_cel_cur = C_DEBRIS;
		debris_cel_prev = C_DEBRIS;
		select_for_rank(pattern_state.pellet_count, 30, 35, 40, 45);
	}

	if(rays.target_y > SEAL_CENTER_Y) {
		if((frame % 2) == 0) {
			rays.unput_and_put(X_LEFT, debris_cel_cur);
		} else if((frame % 2) == 1) {
			rays.unput_and_put(X_RIGHT, debris_cel_cur);
			debris_cel_prev = debris_cel_cur;
			debris_cel_cur = debris_cel_next(debris_cel_cur);
			rays.target_y -= VELOCITY_Y;
		}
		if((frame % 10) == 0) {
			mdrv2_se_play(6);
		}
		return;
	}

	mdrv2_se_play(3);
	rays.unput_broken(VELOCITY_Y);

	for(int i = 0; i < pattern_state.pellet_count; i++) {
		pellets_add_single_rain(
			(PLAYFIELD_LEFT + ((PLAYFIELD_W / pattern_state.pellet_count) * i)),
			SEAL_CENTER_Y,
			irand(),
			1.0f
		);
	}
	frame = 0;
}
