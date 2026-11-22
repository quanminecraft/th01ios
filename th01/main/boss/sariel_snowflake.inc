void near pattern_detonating_snowflake(void)
{
	enum {
		HITBOX_W = 96,
		RADIUS_MAX = (HITBOX_W / 2),
		STEP_OUTER = 0x08,
		STEP_INNER = 0x06,
	};

	enum phase_t {
		P_RESET = -1,
		P_WAND_RAISE = 0,
		P_FLAKE_SPAWN = 1,
		P_FLAKE_MOVING = 2,
		P_DETONATION_START = 3,
		P_DETONATION_ACTIVE = 5,

		_phase_t_FORCE_INT16 = 0x7FFF
	};

	static union {
		phase_t phase;
		int detonation_frame;
	} state = { P_RESET };
	static Subpixel left;
	static screen_x_t star_left;
	static Subpixel top;
	static vram_y_t star_top;
	static Subpixel velocity_y;
	static Subpixel velocity_x;
	static pixel_t radius_outer_1;
	static pixel_t radius_outer_2;
	static pixel_t radius_inner;

	#define ellipse_put(radius_x, radius_y, col, angle_step) { \
		shape_ellipse_arc_put( \
			left.to_pixel(), player_center_y(), radius_x, radius_y, \
			col, angle_step, 0x00, 0xFF \
		); \
	}

	#define ellipse_sloppy_unput(radius_x, radius_y, angle_step) { \
		shape_ellipse_arc_sloppy_unput( \
			left.to_pixel(), player_center_y(), radius_x, radius_y, \
			angle_step, 0x00, 0xFF \
		); \
	}

	if(boss_phase_frame < 10) {
		state.phase = P_RESET;
		select_for_rank(pattern_state.start_frame, 160, 160, 160, 160);
	}

	if((boss_phase_frame % pattern_state.start_frame) == 0) {
		state.phase = P_WAND_RAISE;
	}

	if(state.phase == P_WAND_RAISE) {
		state.phase = static_cast<phase_t>(wand_render_raise_both());
	}
	if(state.phase == P_FLAKE_SPAWN) {
		mdrv2_se_play(6);
		left.v = to_sp(WAND_EMIT_LEFT);
		top.v = to_sp(WAND_EMIT_TOP);
		vector2_between(
			WAND_EMIT_LEFT,
			WAND_EMIT_TOP,
			(player_left + (PLAYER_W / 2) - (FLAKE_W / 2)),
			(player_top + (PLAYER_H / 2) - (FLAKE_H / 2)),
			velocity_x.v,
			velocity_y.v,
			to_sp(7.0f)
		);
		state.phase = P_FLAKE_MOVING;
	}
	if(state.phase == P_FLAKE_MOVING) {
		shape8x8_sloppy_unput(left.to_pixel(), top.to_pixel());

		top.v += velocity_y.v;
		if(top.v >= to_sp(PLAYFIELD_BOTTOM - (FLAKE_H / 2))) {
			state.phase = P_DETONATION_START;
			wand_lower_both();
			return;
		}
		left.v += velocity_x.v;
		shape8x8_flake_put(left.to_pixel(), top.to_pixel(), V_WHITE);
	}
	if(state.phase == P_DETONATION_START) {
		ellipse_put(RADIUS_MAX, 16, COL_DETONATION_START, STEP_OUTER);
		ellipse_put(16, RADIUS_MAX, COL_DETONATION_START, STEP_OUTER);
		ellipse_put(24, 24, V_WHITE, STEP_INNER);
		radius_outer_1 = RADIUS_MAX;
		radius_outer_2 = 16;
		radius_inner = 24;

		// ZUN bug: Assigning a subpixel to a regular pixel. Will affect a
		// single frame, until we enter P_DETONATION_ACTIVE via the
		// [detonation_frame] in the condition below, where the X position is
		// randomized again.
		star_left = left.v;

		star_top = (PLAYFIELD_BOTTOM - (irand() % RADIUS_MAX));
		mdrv2_se_play(10);
	}
	if(state.phase >= P_DETONATION_START) {
		state.detonation_frame++;
	}
	if(
		(state.phase >= P_DETONATION_ACTIVE) &&
		((state.detonation_frame % 8) == 0)
	) {
		shape8x8_sloppy_unput(star_left, star_top);
		ellipse_sloppy_unput(radius_outer_1, radius_outer_2, STEP_OUTER);
		ellipse_sloppy_unput(radius_outer_2, radius_outer_1, STEP_OUTER);
		ellipse_sloppy_unput(radius_inner, radius_inner, STEP_INNER);

		radius_outer_1 -= 6;
		radius_outer_2 += 6;
		radius_inner += 4;
		star_left = ((irand() % ((HITBOX_W * 2) / 3)) + left.to_pixel() - (
			 ((HITBOX_W * 2) / 6) - (FLAKE_W / 2)
		));
		star_top = (PLAYFIELD_BOTTOM - (irand() % RADIUS_MAX));
		if(radius_outer_1 <= 8) {
			state.phase = P_RESET;
			return;
		}

		ellipse_put(radius_outer_1, radius_outer_2, COL_DETONATION, STEP_OUTER);
		ellipse_put(radius_outer_2, radius_outer_1, COL_DETONATION, STEP_OUTER);
		ellipse_put(radius_inner, radius_inner, V_WHITE, STEP_INNER);
		shape8x8_star_put(star_left, star_top, V_WHITE);
	}

	if(
		!player_invincible &&
		((left.to_pixel() + ((HITBOX_W / 2) - (PLAYER_W / 2))) > player_left) &&
		((left.to_pixel() - ((HITBOX_W / 2) + (PLAYER_W / 2))) < player_left) &&
		(state.phase >= P_DETONATION_START)
	) {
		player_is_hit = true;
	}

	#undef ellipse_sloppy_unput
	#undef ellipse_put
}
