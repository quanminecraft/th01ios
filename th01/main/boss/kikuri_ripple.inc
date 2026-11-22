void near ripple_unput_and_put(
	screen_x_t left, vram_y_t top, int ptn_id, int quarter
)
{
	ptn_sloppy_unput_quarter_16(left, top);
	ptn_put_quarter_8(left, top, ptn_id, quarter);
}

void near ripple_update_and_render(
	screen_x_t tear_left, screen_y_t tear_top_max, int8_t &frame
)
{
	// The ripple effect relies entirely on previous cels not being unblitted,
	// and the bottom 8 pixels of the C_RIPPLE_HIGH sprite being flood-filled.
	// Therefore, placing each successive C_RIPPLE_HIGH sprite 8 pixels above
	// the last one entirely avoids the need for C_RIPPLE_FILLED. (Using that
	// sprite would have ensured the stability of the animation against an
	// overlapping player Orb, though!)

	// Expects [ptn_sloppy_unput_before_alpha_put] to be `true`.
	#define unput_and_put(left, top, frame) { \
		ptn_put_quarter_8( \
			left, \
			(top - ( \
				(frame == 0) ? 0 : \
				(frame <= 5) ? (8 * (frame - 1)) : \
				(frame <= 9) ? (8 * (9 - frame)) : \
				0 \
			)), \
			PTN_RIPPLE, \
			( \
				(frame == 0) ? C_RIPPLE_LOW : \
				(frame <= 4) ? C_RIPPLE_HIGH : \
				(frame <= 9) ? C_RIPPLE_LOW : \
				C_RIPPLE_EMPTY /* effectively just unblitting the sprite */ \
			) \
		); \
	}

	#define unput_and_put_center(frame) { \
		unput_and_put((tear_left + ( 0 * RIPPLE_W)), tear_top_max, frame); \
	}
	#define unput_and_put_inner(frame) { \
		unput_and_put((tear_left + (-1 * RIPPLE_W)), tear_top_max, frame); \
		unput_and_put((tear_left + (+1 * RIPPLE_W)), tear_top_max, frame); \
	}
	#define unput_and_put_outer(frame) { \
		unput_and_put((tear_left + (-2 * RIPPLE_W)), tear_top_max, frame); \
		unput_and_put((tear_left + (+2 * RIPPLE_W)), tear_top_max, frame); \
	}

	frame++;

	// ZUN bug: Ripple sprites can be blitted to non-word-aligned X positions,
	// which this hack doesn't support, as explained in its comment. Using such
	// a hack in the first place only makes sense if you unblit and blit each
	// ripple column individually before moving on to the next one. Which the
	// code below does, by symmetrically moving out from the center to the
	// left and right edges. But then, unblitting every successive ripple
	// column by rounding its X coordinate down and up to the nearest word will
	// also cause half of the previously drawn column to be unblitted. This is
	// exactly why most ripple animations show up with weird empty 8-pixel-wide
	// stripes on the inside (→ non-word-aligned X positions), while some do
	// show up fine (→ word-aligned X positions).
	ptn_sloppy_unput_before_alpha_put = true;

	// Ultimate hardcoding! Using entities to track this state just wastes
	// space, right? (Hint: It doesn't, this does.)
	if(frame < 5) {
		unput_and_put_center(0);
		tear_ripple_hittest(tear_left, 0);
		mdrv2_se_play(7);
	} else if(frame < 10) {
		unput_and_put_center(1);
		tear_ripple_hittest(tear_left, 0);
	} else if(frame < 15) {
		tear_ripple_hittest((tear_left - RIPPLE_W), (RIPPLE_W * 2));
		unput_and_put_center(2);
		unput_and_put_inner(0);
	} else if(frame < 20) {
		tear_ripple_hittest((tear_left - RIPPLE_W), (RIPPLE_W * 2));
		unput_and_put_center(3);
		unput_and_put_inner(1);
	} else if(frame < 25) {
		tear_ripple_hittest((tear_left - (RIPPLE_W * 2)), (RIPPLE_W * 4));
		unput_and_put_center(4);
		unput_and_put_inner(2);
		unput_and_put_outer(0);
	} else if(frame < 30) {
		tear_ripple_hittest((tear_left - (RIPPLE_W * 2)), (RIPPLE_W * 4));
		unput_and_put_center(5);
		unput_and_put_inner(3);
		unput_and_put_outer(1);
	} else if(frame < 35) {
		tear_ripple_hittest((tear_left - (RIPPLE_W * 2)), (RIPPLE_W * 4));
		unput_and_put_center(6);
		unput_and_put_inner(4);
		unput_and_put_outer(2);
	} else if(frame < 40) {
		tear_ripple_hittest((tear_left - (RIPPLE_W * 2)), (RIPPLE_W * 4));
		unput_and_put_center(7);
		unput_and_put_inner(5);
		unput_and_put_outer(3);
	} else if(frame < 45) {
		tear_ripple_hittest((tear_left - (RIPPLE_W * 2)), (RIPPLE_W * 4));
		unput_and_put_center(8);
		unput_and_put_inner(6);
		unput_and_put_outer(4);
	} else if(frame < 50) {
		tear_ripple_hittest((tear_left - (RIPPLE_W * 2)), (RIPPLE_W * 4));
		unput_and_put_center(9);
		unput_and_put_inner(7);
		unput_and_put_outer(5);
	} else if(frame < 55) {
		tear_ripple_hittest((tear_left - (RIPPLE_W * 2)), (RIPPLE_W * 4));
		unput_and_put_center(10);
		unput_and_put_inner(8);
		unput_and_put_outer(6);
	} else if(frame < 60) {
		tear_ripple_hittest((tear_left - (RIPPLE_W * 2)), (RIPPLE_W * 4));
		unput_and_put_inner(9);
		unput_and_put_outer(7);
	} else if(frame < 65) {
		unput_and_put_inner(10);
		unput_and_put_outer(8);
	} else if(frame < 70) {
		unput_and_put_outer(9);
	} else if(frame < 75) {
		unput_and_put_outer(10);
	} else {
		frame = 0;
	}

	ptn_sloppy_unput_before_alpha_put = false;

	#undef unput_and_put_outer
	#undef unput_and_put_inner
	#undef unput_and_put_center
	#undef unput_and_put
}

