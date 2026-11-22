inline pixel_t pillar_sprite_row(pixel_t y) {
	return ((PILLAR_SEGMENT_H - 1) - (y % PILLAR_SEGMENT_H));
}

// ZUN bug: Called with non-byte-aligned X positions, creating a discrepancy
// between the on-screen sprite and the hitbox derived from the unaligned
// internal position. (Same as Kikuri's tear ripple sprites.)
void pillar_put_8(screen_x_t left, vram_y_t bottom, pixel_t h)
{
	pixel_t y;
	vram_offset_t vo = vram_offset_shift(left, bottom);

	grcg_setcolor_rmw(COL_PILLAR);

	// Segments
	y = 0;
	while(y < (h - PILLAR_SEGMENT_H)) {
		grcg_put(vo, sPILLAR[0][pillar_sprite_row(y)], PILLAR_W);
		y++;
		vo -= ROW_SIZE;
	}

	// Top part
	y = 0;
	while(y < PILLAR_SEGMENT_H) {
		grcg_put(vo, sPILLAR[1][pillar_sprite_row(y)], PILLAR_W);
		y++;
		vo -= ROW_SIZE;
	}
	grcg_off_func();
}

void pattern_pillars_and_aimed_spreads(void)
{
	enum {
		PILLAR_COUNT = 8,
		DELAY_PER_CIRCLE = 20,
		CIRCLE_ANGLE_STEP = 0x04,

		PILLAR_FRAMES = 16,
		PILLAR_SEGMENTS_PER_FRAME = 2, // (Only rendered every 8 though.)
		PILLAR_SEGMENTS_INITIAL = 4,

		// We (sadly) only render pillars on frames 0 and 8, which means that
		// we miss out on half the height. ZUN already accounted for that and
		// reduced the final unblit height accordingly.
		PILLAR_SEGMENTS_TOTAL = (PILLAR_SEGMENTS_INITIAL - PILLAR_COUNT + (
			PILLAR_FRAMES * PILLAR_SEGMENTS_PER_FRAME
		)),
		PILLAR_UNBLIT_H = (PILLAR_SEGMENTS_TOTAL * PILLAR_SEGMENT_H),

		KEYFRAME_PREPARE = 50,
		KEYFRAME_CIRCLES = 100,
		KEYFRAME_CIRCLE_LAST = (
			KEYFRAME_CIRCLES + ((PILLAR_COUNT - 1) * DELAY_PER_CIRCLE)
		),

		TIME_CIRCLES = 128, // doubles as the circle radius until...
		TIME_PILLARS = 32,
		TIME_PILLARS_DONE = (TIME_PILLARS - PILLAR_FRAMES),
	};

	static struct {
		int time[PILLAR_COUNT];
		screen_x_t center_x[PILLAR_COUNT];
		screen_y_t bottom[PILLAR_COUNT]; // could have been a constant

		int first_circle_frame_for(int i) const {
			return (KEYFRAME_CIRCLES + (i * DELAY_PER_CIRCLE));
		}

		screen_x_t left(int i) const {
			return (center_x[i] - (PILLAR_W / 2));
		}

		pixel_t pillar_h(int i) const {
			return (PILLAR_SEGMENT_H * PILLAR_SEGMENTS_PER_FRAME * (
				(TIME_PILLARS + (PILLAR_SEGMENTS_INITIAL / 2)) - time[i]
			));
		}
	} ent;
	int i;

	#define is_circle_frame_for(i) ( \
		((boss_phase_frame % PILLAR_COUNT) == i) && \
		(boss_phase_frame > ent.first_circle_frame_for(i)) \
	)

	if(boss_phase_frame == KEYFRAME_PREPARE) {
		mima_put_cast_both();
	}
	if(boss_phase_frame < KEYFRAME_CIRCLES) {
		return;
	}
	if(boss_phase_frame == KEYFRAME_CIRCLES) {
		for(i = 0; i < PILLAR_COUNT; i++) {
			ent.time[i] = TIME_CIRCLES;
			ent.bottom[i] = PLAYFIELD_BOTTOM;
		}

		ent.center_x[0] = playfield_rand_x(0.025f, 0.100f);

		// Translation: (playfield_rand_x(0.925f, 1.000f) - (PILLAR_W / 32)).
		// This is the only randomized coordinate that could have possibly come
		// close to result in a pillar position that exceeds the width of VRAM,
		// and pillar_put_8() doesn't clip anything. Therefore, the above
		// translation calculates a random position at the right edge of VRAM,
		// and then shifts it over to make sure that the sprite fits.
		ent.center_x[1] = ((PLAYFIELD_RIGHT - PILLAR_W) -
			playfield_rand_x(0.0f, 0.075f) + (PILLAR_W / 2)
		);

		ent.center_x[2] = playfield_rand_x(0.100f, 0.450f);
		ent.center_x[3] = playfield_rand_x(0.525f, 0.875f);
		ent.center_x[4] = playfield_rand_x(0.200f, 0.450f);
		ent.center_x[5] = playfield_rand_x(0.525f, 0.775f);
		ent.center_x[6] = playfield_rand_x(0.100f, 0.450f);
		ent.center_x[7] = playfield_rand_x(0.525f, 0.875f);

		// That's the same values as in the next pattern…?
		select_for_rank(pattern_state.unused, 40, 45, 50, 55);
		mdrv2_se_play(8);
	}

	// Unblit and update
	for(i = 0; i < PILLAR_COUNT; i++) {
		if(is_circle_frame_for(i) && (ent.time[i] > TIME_PILLARS)) {
			shape_circle_sloppy_unput(
				ent.center_x[i], ent.bottom[i], ent.time[i], CIRCLE_ANGLE_STEP
			);
			ent.time[i] -= PILLAR_COUNT;
		}
	}

	// MODDERS: if(
	// 	(boss_phase_frame <= KEYFRAME_CIRCLES_LAST) &&
	// 	((boss_phase_frame % DELAY_PER_CIRCLE) == 0)
	// )
	if(
		(boss_phase_frame == (KEYFRAME_CIRCLES + (0 * DELAY_PER_CIRCLE))) ||
		(boss_phase_frame == (KEYFRAME_CIRCLES + (1 * DELAY_PER_CIRCLE))) ||
		(boss_phase_frame == (KEYFRAME_CIRCLES + (2 * DELAY_PER_CIRCLE))) ||
		(boss_phase_frame == (KEYFRAME_CIRCLES + (3 * DELAY_PER_CIRCLE))) ||
		(boss_phase_frame == (KEYFRAME_CIRCLES + (4 * DELAY_PER_CIRCLE))) ||
		(boss_phase_frame == (KEYFRAME_CIRCLES + (5 * DELAY_PER_CIRCLE))) ||
		(boss_phase_frame == (KEYFRAME_CIRCLES + (6 * DELAY_PER_CIRCLE))) ||
		(boss_phase_frame == (KEYFRAME_CIRCLES + (7 * DELAY_PER_CIRCLE)))
	) {
		mdrv2_se_play(12);
	}
	if(boss_phase_frame == KEYFRAME_CIRCLE_LAST) {
		meteor_activate();
	}

	// Render and detect collisions
	for(i = 0; i < PILLAR_COUNT; i++) {
		if(!is_circle_frame_for(i)) {
			continue;
		}
		if(ent.time[i] > TIME_PILLARS) {
			shape_circle_put(
				ent.center_x[i],
				ent.bottom[i],
				ent.time[i],
				V_WHITE,
				CIRCLE_ANGLE_STEP
			);

			graph_r_line(
				// Should maybe have been calculated from the entity?
				BASE_CENTER_X,
				BASE_CENTER_Y,

				ent.center_x[i],
				ent.bottom[i],
				COL_SPAWNRAY
			);
		} else if(ent.time[i] > TIME_PILLARS_DONE) {
			if(ent.time[i] == TIME_PILLARS) {
				mdrv2_se_play(7);
				graph_r_line_unput(
					BASE_CENTER_X, BASE_CENTER_Y, ent.center_x[i], ent.bottom[i]
				);
			}
			pillar_put_8(ent.left(i), ent.bottom[i], ent.pillar_h(i));

			// Translation: Reimu's center point has to be at least
			// (32 - 8) = 24 pixels away from the pillar's center.
			if(
				!player_invincible &&
				(player_left > (ent.center_x[i] - PLAYER_W - (PILLAR_W / 4))) &&
				(player_left < (ent.center_x[i] + (PILLAR_W / 4)))
			) {
				player_is_hit = true;
			}
			ent.time[i] -= PILLAR_COUNT;
		} else if(ent.time[i] != PIXEL_NONE) {
			Pellets.add_group(
				ent_still.cur_center_x(),
				ent_still.cur_center_y(),
				PG_3_SPREAD_WIDE_AIMED,
				to_sp(2.25f)
			);
			egc_copy_rect_1_to_0_16(
				ent.left(i),
				(PLAYFIELD_BOTTOM - PILLAR_UNBLIT_H),
				PILLAR_W,
				PILLAR_UNBLIT_H
			);
			ent.time[i] = PIXEL_NONE;
		}
	}
	// MODDERS: Loop, obviously.
	if(
		(ent.time[0] == PIXEL_NONE) && (ent.time[1] == PIXEL_NONE) &&
		(ent.time[2] == PIXEL_NONE) && (ent.time[3] == PIXEL_NONE) &&
		(ent.time[4] == PIXEL_NONE) && (ent.time[5] == PIXEL_NONE) &&
		(ent.time[6] == PIXEL_NONE) && (ent.time[7] == PIXEL_NONE)
	) {
		boss_phase_frame = 0;
	}

	#undef is_circle_frame_for
}

