inline pixel_t spreadin_bottom_cur(void) {
	return ((spreadin_speed / spreadin_interval) * (boss_phase_frame - 10));
}

static const int KEYFRAME_SPREADIN_START = 10;

// Renders a frame of the spread-in animation that ultimately ends in a full
// Mima at the given position. Sets [boss_phase_frame] to 0 to indicate that
// the animation completed.
void phase_spreadin(screen_x_t final_left, screen_y_t final_top)
{
	pixel_t row;
	pixel_t line_on_top;

	if(boss_phase_frame < KEYFRAME_SPREADIN_START) {
		return;
	} else if(boss_phase_frame == KEYFRAME_SPREADIN_START) {
		ent_still.pos_cur_set(final_left, final_top);
		mima_bg_snap();
		line_on_top = (final_top + (MIMA_H / 2));
		return;
	} else if((boss_phase_frame % spreadin_interval) != 0) {
		return;
	}

	line_on_top = ((MIMA_H / 2) - spreadin_bottom_cur());
	if(line_on_top < 0) {
		boss_phase_frame = 0;
		mima_put_still_both();
		return;
	}
	for(row = 0; spreadin_bottom_cur() > row; row++) {
		ent_still.unput_and_put_1line(
			final_left, (final_top + line_on_top + row), ent_still.image(), row
		);
		ent_still.unput_and_put_1line(
			final_left,
			((final_top + MIMA_H) - line_on_top - row),
			ent_still.image(),
			((MIMA_H - 1) - row)
		);
	}
}

