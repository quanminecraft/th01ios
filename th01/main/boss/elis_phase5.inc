elis_phase_5_subphase_t phase_5_girl(bool16 reset = false)
{
	#define star_of_david_then(in, pattern_cur, func) { \
		if(subphase == SP_STAR_OF_DAVID) { \
			subphase = star_of_david(); \
			return P5_PATTERN; \
		} \
		/* subphase == SP_PATTERN */ \
		subphase = func(); \
		if(subphase == SP_STAR_OF_DAVID) { \
			subphase = SP_STAR_OF_DAVID; \
			pattern_cur = CHOOSE_NEW; \
			return P5_TRANSFORM; \
		} \
		return P5_PATTERN; \
	}

	static int pattern_cur = CHOOSE_NEW;
	static elis_starpattern_ret_t subphase = SP_STAR_OF_DAVID;

	if(reset == true) {
		pattern_cur = CHOOSE_NEW;
		subphase = SP_STAR_OF_DAVID;
		return P5_PATTERN;
	}

	#ifdef TH01_NATIVE_ORB_PHYSICS
    native_elis_observe_girl(pattern_cur);
#endif
switch(pattern_cur) {
	case CHOOSE_NEW:
		// In contrast to phases 1 and 3, no pattern cycle is skipped here.
		pattern_cur = ((irand() % 3) + 1);
		break;
	case 1: /* return */ star_of_david_then(subphase, pattern_cur,
		pattern_three_symmetric_4_stacks_then_symmetric_arc
	);
	case 2: /* return */ star_of_david_then(subphase, pattern_cur,
		pattern_safety_circle_and_rain_from_top
	);
	case 3: /* return */ star_of_david_then(subphase, pattern_cur,
		pattern_aimed_5_spreads_and_lasers_followed_by_ring
	);
	}
	return P5_PATTERN;

	#undef star_of_david_then
}

void phase_5(
	elis_form_t& form,
	pixel_t& bat_velocity_x,
	pixel_t& bat_velocity_y,
	bool16 reset = false
)
{
	static elis_phase_5_subphase_t subphase = P5_PATTERN;
	static int pattern_bat_cur = CHOOSE_NEW;

	if(reset == true) {
		subphase = P5_PATTERN;
		pattern_bat_cur = CHOOSE_NEW;
		phase_5_girl(true);
		return;
	}
	if(form == F_BAT) {
		if(subphase == P5_PATTERN) {
			subphase = bat_fly_random(bat_velocity_x, bat_velocity_y);
			if(pattern_bat_cur == CHOOSE_NEW) {
				// In contrast to phases 1 and 3, no pattern cycle is skipped
				// here.
				pattern_bat_cur = ((irand() % 4) + 1);
			}
			switch(pattern_bat_cur) {
			case 1:
				pattern_bat_slow_spreads();
				break;
			case 2:
				pattern_bat_alternating_narrow_and_wide_2_spreads();
				break;
			case 3:
				pattern_bat_random_rain();
				break;
			case 4:
				pattern_bat_alternating_2_and_3_spreads();
				break;
			}
		} else if(subphase == P5_TRANSFORM) { // Thanks for making it explicit!
			form = transform_bat_to_girl();
			if(form == F_GIRL) {
				pattern_bat_cur = CHOOSE_NEW;
				subphase = P5_PATTERN;
			}
		}
	} else /* if(form == F_GIRL) */ {
		if(subphase == P5_PATTERN) {
			subphase = phase_5_girl();
		} else /* if(subphase == P5_TRANSFORM) */ {
			form = transform_girl_to_bat();
			if(form == F_BAT) {
				subphase = P5_PATTERN;
			}
		}
	}
#ifdef TH01_NATIVE_ORB_PHYSICS
    native_elis_observe_final(subphase,pattern_bat_cur);
#endif
}

