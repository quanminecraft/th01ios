static const int MISSILE_INTERVAL = 8;

// The original code generation of missile_angle() combines all constant parts
// into a single immediate argument, so these can't be defined in terms of
// CEyeEntity's cur_center_x() and cur_center_y() helpers.

// (cur_center_x() + 4) kind of corresponds to the right edge of C_DOWN.
static const pixel_t MISSILE_OFFSET_LEFT = ((EYE_W / 2) + 4 - (MISSILE_W / 2));

// (pupil_bottom() - (MISSILE_H / 2))
static const pixel_t MISSILE_OFFSET_TOP = (
	(EYE_H / 2) + PUPIL_H - (MISSILE_H / 2)
);

inline unsigned char missile_angle(
	const CEyeEntity& eye, const screen_x_t& target_x
) {
	return iatan2(
		(player_bottom() - eye.cur_top  + MISSILE_OFFSET_TOP),
		(target_x        - eye.cur_left + MISSILE_OFFSET_LEFT)
	);
}

void near fire_missile_pair_from_south(
	unsigned char angle_for_southwest, unsigned char angle_for_southeast
)
{
	point_t velocity;

	vector2(velocity.x, velocity.y, 8, angle_for_southwest);
	Missiles.add(
		(eye_southwest.cur_left + MISSILE_OFFSET_LEFT),
		(eye_southwest.cur_top  + MISSILE_OFFSET_TOP),
		velocity.x,
		velocity.y
	);
	vector2(velocity.x, velocity.y, 8, angle_for_southeast);
	Missiles.add(
		(eye_southeast.cur_left + MISSILE_OFFSET_LEFT),
		(eye_southeast.cur_top  + MISSILE_OFFSET_TOP),
		velocity.x,
		velocity.y
	);
}

enum missile_subphase_t {
	MSP_PREPARE = 0,
	MSP_AIM_AND_FIRE_AROUND = 1,
	MSP_SHIFT_ANGLE_1 = 2,
	MSP_SHIFT_ANGLE_2 = 3,
	MSP_COUNT = 4,
};

#define missile_subphase_variation(variation, msp) ( \
	(variation * MSP_COUNT) + msp \
)

// MODDERS: Simply replace with ((subphase / MSP_COUNT) == variation).
#define missile_subphase_variation_is( \
	subphase, variation, msp, variations_max \
) ( \
	(variations_max >= (variation + 1)) && \
	(subphase == missile_subphase_variation(variation, msp)) \
)

// MODDERS: Simply replace with ((subphase % MSP_COUNT) == msp).
#define missile_subphase_is(subphase, msp, variations_max) ( \
	missile_subphase_variation_is(subphase, 0, msp, variations_max) || \
	missile_subphase_variation_is(subphase, 1, msp, variations_max) || \
	missile_subphase_variation_is(subphase, 2, msp, variations_max) \
)

inline void missile_pairs_shift_angle_1_away(
	const int& subphase,
	unsigned char& angle_southwest,
	unsigned char& angle_southeast
) {
	angle_southwest -= 0x02;
	angle_southeast += 0x02;
}

#define missile_pairs_shift_angle_2_towards( \
	subphase, angle_southwest, angle_southeast \
) { \
	if(subphase == missile_subphase_variation(0, MSP_SHIFT_ANGLE_2)) { \
		angle_southwest += 0x04; \
	} else if(subphase == missile_subphase_variation(1, MSP_SHIFT_ANGLE_2)) { \
		angle_southeast -= 0x04;\
	} else if(subphase == missile_subphase_variation(2, MSP_SHIFT_ANGLE_2)) { \
		angle_southwest += 0x02; \
		angle_southeast -= 0x02; \
	} \
}

inline void missile_pairs_shift_angle_1_clock(
	const int& subphase,
	unsigned char& angle_southwest,
	unsigned char& angle_southeast
) {
	if(subphase == missile_subphase_variation(0, MSP_SHIFT_ANGLE_1)) {
		angle_southwest -= 0x02;
		angle_southeast -= 0x02;
	} else {
		angle_southwest += 0x02;
		angle_southeast += 0x02;
	}
}

#define missile_pairs_shift_angle_2_clock( \
	subphase, angle_southwest, angle_southeast \
) { \
	if(subphase == missile_subphase_variation(0, MSP_SHIFT_ANGLE_2)) { \
		angle_southwest += 0x03; \
		angle_southeast += 0x03; \
	} else if(subphase == missile_subphase_variation(1, MSP_SHIFT_ANGLE_2)) { \
		angle_southwest -= 0x03; \
		angle_southeast -= 0x03; \
	} \
}

// Processes a single frame of the missile pair pattern. [subphase] cycles
// between the fields of missile_subphase_t, offset by randomly selected
// variations. These can be differentiated in [shift_angle_1_func] and
// [shift_angle_2_func].
#define pattern_missile_pairs_from_south( \
	subphase, \
	missile_pairs_fired_in_subphase, \
	target_left, \
	angle_southwest, \
	angle_southeast, \
	iterations_done, \
	variations_max, \
	shift_angle_1_func, \
	shift_angle_2_func \
) { \
	eyes_locked_unput_and_put_then_track(EF_SOUTHWEST | EF_SOUTHEAST); \
	\
	if(boss_phase_frame >= (SUBPHASE_PREPARE_FRAMES - 10)) { \
		/** \
		 * Hardcoding two maximum variations? Doesn't matter though: \
		 * If we're in MSP_PREPARE, we're always in variation 0 (see below). \
		 */ \
		if(missile_subphase_is(subphase, MSP_PREPARE, 2)) { \
			eye_southwest.set_image(C_CLOSED); \
			eye_southeast.set_image(C_CLOSED); \
		} \
	} \
	if( \
		(boss_phase_frame == SUBPHASE_PREPARE_FRAMES) || \
		missile_subphase_is(subphase, MSP_AIM_AND_FIRE_AROUND, variations_max) \
	) { \
		if(missile_subphase_variation_is(subphase, 0, MSP_PREPARE, 1)) { \
			subphase = missile_subphase_variation( \
				(irand() % variations_max), MSP_AIM_AND_FIRE_AROUND \
			); \
			target_left = (player_left - (MISSILE_W / 2)); \
			angle_southwest = missile_angle(eye_southwest, target_left); \
			angle_southeast = missile_angle(eye_southeast, target_left); \
			angle_southwest -= 0x10; \
			angle_southeast += 0x10; \
		} \
		if((boss_phase_frame % MISSILE_INTERVAL) == 0) { \
			fire_missile_pair_from_south(angle_southwest, angle_southeast); \
		} \
		if(boss_phase_frame == SUBPHASE_TIMEOUT_FRAMES) { \
			subphase++; \
			missile_pairs_fired_in_subphase = 0; \
		} \
	} \
	if(missile_subphase_is(subphase, MSP_SHIFT_ANGLE_1, variations_max)) { \
		if((boss_phase_frame % MISSILE_INTERVAL) == 1) { \
			missile_pairs_fired_in_subphase++; \
			shift_angle_1_func(subphase, angle_southwest, angle_southeast); \
			fire_missile_pair_from_south(angle_southwest, angle_southeast); \
			if(missile_pairs_fired_in_subphase > 10) { \
				subphase++; \
				missile_pairs_fired_in_subphase = 0; \
			} \
		} \
	} else if( \
		missile_subphase_is(subphase, MSP_SHIFT_ANGLE_2, variations_max) \
	) { \
		if((boss_phase_frame % MISSILE_INTERVAL) == 1) { \
			missile_pairs_fired_in_subphase++; \
			shift_angle_2_func(subphase, angle_southwest, angle_southeast); \
			fire_missile_pair_from_south(angle_southwest, angle_southeast); \
			if(missile_pairs_fired_in_subphase > 10) { \
				/** \
				 * Yup, always resetting to variation 0, which will in turn \
				 * select a new random one. \
				 */ \
				subphase = missile_subphase_variation(0, MSP_PREPARE); \
				\
				missile_pairs_fired_in_subphase = 0; \
				boss_phase_frame = 0; \
				iterations_done++; \
			} \
		} \
	} \
}
// --------------

