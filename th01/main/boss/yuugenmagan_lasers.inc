static const pixel_t LASER_W = 3;
static const pixel_t LASER_VELOCITY = 4;
static const int LASER_INTERVAL = 4;
static const pixel_t LASER_VELOCITY_STEP = (LASER_VELOCITY * LASER_INTERVAL);
static const dots8_t LASER_DOTS = (
	((1 << LASER_W) - 1) << (BYTE_DOTS - LASER_W) // (***_____)
);

enum laser_subphase_t {
	LSP_PREPARE = 0,
	LSP_ACTIVE = 1,
	LSP_DONE = 2,
};

inline int laser_subphase(int iteration, laser_subphase_t lsp) {
	return ((iteration * LSP_DONE) + lsp);
}

#define laser_pattern_should_run(subphase, iteration) ( \
	( \
		(boss_phase_frame == 40) && \
		(subphase == laser_subphase(iteration, LSP_PREPARE)) \
	) || (subphase == laser_subphase(iteration, LSP_ACTIVE)) \
)

#define laser_eye eye_north

inline screen_x_t laser_left(void) {
	return laser_eye.offcenter_x();
}

inline screen_y_t laser_top(void) {
	return laser_eye.iris_top();
}

inline screen_x_t laser_right(const pixel_t& x_edge_offset, x_direction_t dir) {
	return ((dir == X_LEFT)
		? (PLAYFIELD_RIGHT - x_edge_offset)
		: (PLAYFIELD_LEFT + x_edge_offset)
	);
}

inline void laser_unput_render_hittest(
	const pixel_t& x_edge_offset, x_direction_t dir
) {
	screen_x_t cur_right = laser_right(x_edge_offset, dir);
	screen_y_t cur_top = laser_top();
	screen_y_t cur_left = laser_left();

	screen_x_t prev_right = (laser_right(x_edge_offset, dir) - (
		dir ? -LASER_VELOCITY_STEP : LASER_VELOCITY_STEP
	));
	screen_y_t prev_top = laser_top();
	screen_y_t prev_left = laser_left();

	linebullet_unput(prev_left, prev_top, prev_right, PLAYFIELD_BOTTOM);
	linebullet_put_patterned_and_hittest(
		cur_left, cur_top, cur_right, PLAYFIELD_BOTTOM, V_WHITE, LASER_DOTS
	);
}

inline void laser_unput(const pixel_t& x_edge_offset, x_direction_t dir) {
	linebullet_unput(
		laser_left(),
		laser_top(),
		laser_right(x_edge_offset, dir),
		PLAYFIELD_BOTTOM
	);
}

// Separate function to work around the `Condition is always true/false` and
// `Unreachable code` warnings. Should really be done unconditionally, though.
inline void conditionally_reset(pixel_t& x_edge_offset, bool cond) {
	if(cond) {
		x_edge_offset = 0;
	}
}

#define pattern_single_laser_across_playfield( \
	subphase, redundant_lvalue, x_edge_offset, iteration, right_to_left \
) \
	if(subphase == laser_subphase(iteration, LSP_PREPARE)) { \
		subphase = laser_subphase(iteration, LSP_ACTIVE); \
		redundant_lvalue = 0; /* ZUN bloat */ \
		conditionally_reset(x_edge_offset, right_to_left); \
	} \
	if((boss_phase_frame % LASER_INTERVAL) == 1) { \
		/* Track the laser target */ \
		if(x_edge_offset <= (PLAYFIELD_CENTER_X - EYE_W)) { \
			laser_eye.set_image(right_to_left ? C_RIGHT : C_LEFT); \
		} else if(x_edge_offset >= (PLAYFIELD_CENTER_X + EYE_W)) { \
			laser_eye.set_image(right_to_left ? C_LEFT : C_RIGHT); \
		} else { \
			laser_eye.set_image(C_DOWN); \
		} \
		\
		laser_unput_render_hittest(x_edge_offset, right_to_left); \
		if(x_edge_offset >= (PLAYFIELD_W - (PLAYER_W + (PLAYER_W / 2)))) { \
			laser_unput(x_edge_offset, right_to_left); \
			subphase = laser_subphase((iteration + 1), LSP_PREPARE); \
			boss_phase_frame = 0; \
			\
			/**
			 * ZUN bloat: Done after LSP_PREPARE, where it's much more \
			 * appropriate. \
			 */ \
			x_edge_offset = 0; \
			\
			if(rank > RANK_NORMAL) { \
				laser_eye.fire_from_iris_center( \
					PG_3_SPREAD_NARROW_AIMED, 0.1875f \
				); \
			} \
		} \
		x_edge_offset += LASER_VELOCITY_STEP; \
	}

#define pattern_dual_lasers_across_playfield( \
	subphase, redundant_lvalue, x_edge_offset, iteration \
) \
	if(subphase == laser_subphase(iteration, LSP_PREPARE)) { \
		subphase = laser_subphase(iteration, LSP_ACTIVE); \
		redundant_lvalue = 0; /* ZUN bloat */ \
		x_edge_offset = 0; \
	} \
	if((boss_phase_frame % LASER_INTERVAL) == 1) { \
		laser_eye.set_image(C_AHEAD); \
		\
		/** \
		 * ZUN bug: The really should be both unblitted and *then* both \
		 * rendered. With graph_r_line_unput() unblitting 32 horizontal \
		 * pixels for every row, the top part of the X_RIGHT-moving laser \
		 * will never be fully visible. \
		 */ \
		laser_unput_render_hittest(x_edge_offset, X_RIGHT); \
		laser_unput_render_hittest(x_edge_offset, X_LEFT); \
		if(x_edge_offset >= (PLAYFIELD_CENTER_X - PLAYER_W)) { \
			laser_unput(x_edge_offset, X_RIGHT); \
			laser_unput(x_edge_offset, X_LEFT); \
			subphase = laser_subphase(iteration, LSP_DONE); \
			boss_phase_frame = 0; \
			x_edge_offset = 0; \
			if(rank == RANK_HARD) { \
				laser_eye.fire_from_iris_center( \
					PG_3_SPREAD_NARROW_AIMED, 0.1875f \
				); \
			} \
			if(rank == RANK_LUNATIC) { \
				laser_eye.fire_from_iris_center( \
					PG_5_SPREAD_NARROW_AIMED, 0.25f \
				); \
			} \
		} \
		x_edge_offset += LASER_VELOCITY_STEP; \
	}
// ---------------------------

