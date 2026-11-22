#define rifts_update_and_render( \
	rifts, ent, start_frame, end_frame, col, tmp_cel \
) { \
	if( \
		(boss_phase_frame >= start_frame) && \
		(boss_phase_frame <= end_frame) && \
		((boss_phase_frame % 4) == 0) \
	) { \
		for(int i = 0; i < rifts.count(); i++) { \
			/* Unblit */ \
			if(boss_phase_frame > start_frame) { \
				elis_grc_sloppy_unput(rifts, i); \
			} \
			\
			/* Initialize */ \
			if( \
				((boss_phase_frame % 16) == ((i * 4) % 16)) || \
				(boss_phase_frame == start_frame) \
			) { \
				rifts.left[i] = surround_random_left(ent); \
				rifts.top[i] = surround_random_top(ent); \
			} \
			\
			/* Render */ \
			if(boss_phase_frame < end_frame) { \
				tmp_cel = (irand() % RIFT_CELS); \
				elis_grc_put(rifts, i, (C_RIFT + cel), col); \
			} \
		} \
		mdrv2_se_play(7); \
	} \
}
// -------------

