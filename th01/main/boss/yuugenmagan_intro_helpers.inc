static const int EYE_OPENING_FRAMES = 60;
static const int EYE_TOGGLE_FRAMES = (EYE_OPENING_FRAMES + 10);
static const int SUBPHASE_PREPARE_FRAMES = 100;
static const int SUBPHASE_TIMEOUT_FRAMES = 200;

enum phase_0_keyframe_t {
	KEYFRAME_LATERAL_OPENING = SUBPHASE_PREPARE_FRAMES,
	KEYFRAME_SOUTH_OPENING = 120,
	KEYFRAME_NORTH_OPENING = 140,
	KEYFRAME_LATERAL_OPEN = (KEYFRAME_LATERAL_OPENING + EYE_OPENING_FRAMES),
	KEYFRAME_SOUTH_OPEN = (KEYFRAME_SOUTH_OPENING + EYE_OPENING_FRAMES),
	KEYFRAME_NORTH_OPEN = (KEYFRAME_NORTH_OPENING + EYE_OPENING_FRAMES),

	KEYFRAME_CLOSING = 240,
	KEYFRAME_CLOSED = 260,
	KEYFRAME_HIDDEN = 280,
	KEYFRAME_LATERAL_LASER_DONE = 300,
	KEYFRAME_SOUTH_LASER_DONE = 320,
	KEYFRAME_NORTH_LASER_DONE = 330,
};

// Matches the animation in eyes_toggle_and_yokoshima_recolor().
inline void phase_0_eye_open(CEyeEntity& eye, int frame, int frame_first) {
	eye.set_image(
		((frame - frame_first) < ((EYE_OPENING_FRAMES / 3) * 1)) ? C_CLOSED :
		((frame - frame_first) < ((EYE_OPENING_FRAMES / 3) * 2)) ? C_HALFOPEN :
		((frame - frame_first) < ((EYE_OPENING_FRAMES / 3) * 3)) ? C_AHEAD :
		/*                                                      */ C_DOWN
	);
}

inline void phase_0_eye_close(CEyeEntity& eye, int frame) {
	eye.set_image(
		(frame == KEYFRAME_CLOSING) ? C_HALFOPEN :
		(frame == KEYFRAME_CLOSED) ? C_CLOSED : C_HIDDEN
	);
}

inline void phase_0_eyes_open(int frame) {
	if(
		(frame >= KEYFRAME_LATERAL_OPENING) &&
		(frame <= KEYFRAME_LATERAL_OPEN)
	) {
		phase_0_eye_open(eye_west, frame, KEYFRAME_LATERAL_OPENING);
		phase_0_eye_open(eye_east, frame, KEYFRAME_LATERAL_OPENING);
	}
	if((frame >= KEYFRAME_SOUTH_OPENING) && (frame <= KEYFRAME_SOUTH_OPEN)) {
		phase_0_eye_open(eye_southwest, frame, KEYFRAME_SOUTH_OPENING);
		phase_0_eye_open(eye_southeast, frame, KEYFRAME_SOUTH_OPENING);
	}
	if((frame >= KEYFRAME_NORTH_OPENING) && (frame <= KEYFRAME_NORTH_OPEN)) {
		phase_0_eye_open(eye_north, frame, KEYFRAME_NORTH_OPENING);
	}
}

inline void phase_0_eyes_close(int frame) {
	phase_0_eye_close(eye_west, frame);
	phase_0_eye_close(eye_east, frame);
	phase_0_eye_close(eye_southwest, frame);
	phase_0_eye_close(eye_southeast, frame);
	phase_0_eye_close(eye_north, frame);
}

void phase_0_downwards_lasers(void)
{
	#define laser_hittest(eye, player_w) ( \
		overlap_low_center_lt_gt( \
			player_left, player_w, eye.offcenter_x(), (EYE_W - (EYE_W / 4)) \
		) \
	)

	if(boss_phase_frame > KEYFRAME_LATERAL_OPEN) {
		if(boss_phase_frame < KEYFRAME_LATERAL_LASER_DONE) {
			eye_west.downwards_laser_put();
			eye_east.downwards_laser_put();

			// ZUN quirk: The hitbox for the rightmost eye is much larger than
			// the other ones?!
			if(
				laser_hittest(eye_west, PLAYER_W) ||
				laser_hittest(eye_east, (PLAYER_W / 4))
			) {
				player_is_hit = true;
			}
		}
		if(boss_phase_frame > KEYFRAME_SOUTH_OPEN) {
			if(boss_phase_frame < KEYFRAME_SOUTH_LASER_DONE) {
				eye_southwest.downwards_laser_put();
				eye_southeast.downwards_laser_put();
				if(
					laser_hittest(eye_southwest, PLAYER_W) ||
					laser_hittest(eye_southeast, PLAYER_W)
				) {
					player_is_hit = true;
				}
			}
			if(boss_phase_frame > KEYFRAME_NORTH_OPEN) {
				eye_north.downwards_laser_put();
				if(laser_hittest(eye_north, PLAYER_W)) {
					player_is_hit = true;
				}
			}
		}
	}
	if(player_invincible) {
		player_is_hit = false;
	}

	#undef laser_hittest
}

inline void phase_0_fire(CEyeEntity& eye) {
	eye.fire_from_iris_center(PG_5_SPREAD_WIDE_AIMED, 3.375f);
}

