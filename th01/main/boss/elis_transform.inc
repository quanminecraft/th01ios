enum {
	TRANSFORM_START_FRAME = 20,
	TRANSFORM_END_FRAME = 100,
};

#define transform_shake(start_frame) { \
	if((boss_phase_frame >= start_frame) && ((boss_phase_frame % 2) == 0) ) { \
		z_vsync_wait_and_scrollup(RES_Y - ((boss_phase_frame % 4) * 2)); \
	} \
}

elis_form_t transform_girl_to_bat(void)
{
	static EntitiesTopleft<5> rifts;
	int cel;  // ACTUAL TYPE: elis_grc_cel_t

	if(boss_phase_frame == TRANSFORM_START_FRAME) {
		ent_unput_and_put_both(ent_attack, 2, C_PREPARE);
	}

	rifts_update_and_render(
		rifts,
		ent_still_or_wave,
		TRANSFORM_START_FRAME,
		TRANSFORM_END_FRAME,
		(irand() % COLOR_COUNT),
		cel
	);
	transform_shake(TRANSFORM_START_FRAME);

	if(boss_phase_frame > TRANSFORM_END_FRAME) {
		ent_bat.pos_cur_set_to(ent_still_or_wave);

		// ZUN bug: No bat sprite rendered this frame? Wouldn't have happened
		// if the original code consistently used the ent_unput_and_put_both()
		// helper function we've added. :P
		ent_bat.set_image(C_BAT);
		page_access(1);	girl_bg_put(2);
		page_access(0);	girl_bg_put(2);

		z_vsync_wait_and_scrollup(RES_Y);
		boss_phase_frame = 0;
		return F_BAT;
	}
	return F_GIRL;
}

elis_form_t transform_bat_to_girl(void)
{
	static EntitiesTopleft<5> rifts;
	int cel;  // ACTUAL TYPE: elis_grc_cel_t
	screen_x_t left;
	screen_y_t top;

	rifts_update_and_render(
		rifts,
		ent_bat,
		TRANSFORM_START_FRAME,
		TRANSFORM_END_FRAME,
		(irand() % COLOR_COUNT),
		cel
	);
	transform_shake(TRANSFORM_START_FRAME);

	if(boss_phase_frame > TRANSFORM_END_FRAME) {
		// Doubly sloppy! CBossEntity has a function for that!
		egc_copy_rect_1_to_0_16(
			ent_bat.cur_left, ent_bat.cur_top, BAT_W, BAT_H
		);

		// Should ideally be a branch in ent_sync(), but that would make the
		// function too complex to perfectly inline. It's only used here,
		// anyway.
		left = (ent_bat.cur_center_x() - (GIRL_W / 2));
		top  = (ent_bat.cur_center_y() - (GIRL_H / 2));
		if(left < PLAYFIELD_LEFT) {
			left = PLAYFIELD_LEFT;
		} else if(left > (PLAYFIELD_RIGHT - GIRL_W)) {
			left = (PLAYFIELD_RIGHT - GIRL_W);
		}
		if(top < PLAYFIELD_TOP) {
			top = PLAYFIELD_TOP;
		}
		ent_still_or_wave.pos_cur_set(left, top);

		girl_bg_snap(1);
		ent_unput_and_put_both(ent_attack, 2, C_ATTACK_1);
		z_vsync_wait_and_scrollup(RES_Y);
		boss_phase_frame = 0;
		return F_GIRL;
	}
	return F_BAT;
}

