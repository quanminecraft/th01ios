int pattern_random_from_rifts(void)
{
	enum {
		KEYFRAME_0 = 60, 	// Firing down
		KEYFRAME_1 = 120,	// Firing in all directions
		KEYFRAME_2 = 200,	// No more firing
		KEYFRAME_3 = 220,	// Pattern done
	};

	static EntitiesTopleft<5> rifts;
	int cel; // ACTUAL TYPE: elis_grc_cel_t
	unsigned char angle;

	ent_attack_render();
	if(boss_phase_frame == (KEYFRAME_0 / 2)) {
		select_for_rank(pattern_state.interval, 6, 3, 2, 2);
	}

	// That's quite the brave placement for this branch...
	if(
		(boss_phase_frame > KEYFRAME_0) &&
		((boss_phase_frame % pattern_state.interval) == 0) &&
		(boss_phase_frame < KEYFRAME_1)
	) {
		int i = (irand() % rifts.count());
		angle = ((irand() % 0x15) + 0x36);
		Pellets.add_single(rifts.left[i], rifts.top[i], angle, to_sp(6.0f));
	} else if(
		(boss_phase_frame >= KEYFRAME_1) && (boss_phase_frame < KEYFRAME_2)
	) {
		int i = (irand() % rifts.count());
		angle = ((irand() % 0x15) + 0x36);
		unsigned char angle_offset = ((irand() & 1)
			? +(boss_phase_frame - KEYFRAME_1)
			: -(boss_phase_frame - KEYFRAME_1)
		);
		angle *= (angle_offset / 2);
		Pellets.add_single(rifts.left[i], rifts.top[i], angle, to_sp(6.0f));
	}

	rifts_update_and_render(
		rifts, ent_still_or_wave, KEYFRAME_0, KEYFRAME_2, COL_FX, cel
	);

	if(boss_phase_frame > KEYFRAME_3) {
		boss_phase_frame = 0;
		return CHOOSE_NEW;
	}
	return 3;
}

