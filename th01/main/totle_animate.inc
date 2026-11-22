inline void totle_metric_4_digit_animate(totle_value_t& val, screen_y_t top) {
	totle_metric_digit_animate((val / 1000), 0, top);	val = (val % 1000);
	totle_metric_digit_animate((val / 100), 1, top); 	val = (val % 100);
	totle_metric_digit_animate((val / 10), 2, top);  	val = (val % 10);
	totle_metric_digit_animate(val, 3, top);
	totle_metric_digit_animate(0, 4, top);
	frame_delay(5);
}

inline void totle_metric_5_digit_animate(totle_value_t& val, screen_y_t top) {
	totle_metric_digit_animate((val / 10000), -1, top);	val = (val % 10000);
	totle_metric_digit_animate((val / 1000), 0, top);  	val = (val % 1000);
	totle_metric_digit_animate((val / 100), 1, top);   	val = (val % 100);
	totle_metric_digit_animate((val / 10), 2, top);

	// MODDERS: Don't assume this one to be 0.
	totle_metric_digit_animate(0, 3, top);

	totle_metric_digit_animate(0, 4, top);
	frame_delay(5);
}

void totle_animate(int stage_num)
{
	unsigned int i;
	int bonus_remainder;
	int bonus_div_10000;

	Shots.unput_and_reset();
	Pellets.unput_and_reset_nonclouds();
	frame_delay(50);

	totle_value_t val = 0;

	totle_load_and_pagetrans_animate();

	ptn_put_8(
		(TOTLE_STAGE_LEFT + (0 * PTN_W)),
		TOTLE_STAGE_TOP,
		(PTN_TOTLE_NUMERAL_32 + (stage_num / 10))
	);
	ptn_put_8(
		(TOTLE_STAGE_LEFT + (1 * PTN_W)),
		TOTLE_STAGE_TOP,
		(PTN_TOTLE_NUMERAL_32 + (stage_num % 10))
	);
	frame_delay(30);

	// Time
	#ifdef TH01_NATIVE_ORB_PHYSICS
	val = static_cast<std::uint16_t>(stage_timer * 5u);
#else
	val = (stage_timer * 5);
#endif
	clamp_and_add_x10_to_score_bonus(val, 6553, i);
	totle_metric_4_digit_animate(val, TOTLE_METRIC_TIME_TOP);

	// Card combo ("Continuous")
	// There are no cards to be flipped in boss stages though?
	#ifdef TH01_NATIVE_ORB_PHYSICS
	val = th01_native::bonus_dos_int(cardcombo_max * 200);
#else
	val = (cardcombo_max * 200);
#endif
	clamp_and_add_x10_to_score_bonus(val, 6553, i);
	totle_metric_4_digit_animate(val, TOTLE_METRIC_CARDCOMBO_MAX_TOP);

	// Bomb & Player ("MIKOsan")
	val = (rem_lives * 500);
	val += (rem_bombs * 200);
	clamp_and_add_x10_to_score_bonus(val, 6553, i);
	totle_metric_4_digit_animate(val, TOTLE_METRIC_RESOURCES_TOP);

	// Stage number
	// Obviously requires a higher clamping factor.
	val = (stage_num * 1000);
	clamp_and_add_x10_to_score_bonus(val, 65530, i);
	totle_metric_5_digit_animate(val, TOTLE_METRIC_STAGE_NUMBER_TOP);

	// Commit
	score += score_bonus;

	// TOTLE
	bonus_div_10000 = (score_bonus / 10000);
	bonus_remainder = (score_bonus - (bonus_div_10000 * 10000));
	totle_metric_digit_animate((bonus_div_10000 / 10),  -1, TOTLE_TOTLE_TOP);
	bonus_div_10000 = (bonus_div_10000 % 10);
	totle_metric_digit_animate(bonus_div_10000,          0, TOTLE_TOTLE_TOP);
	totle_metric_digit_animate((bonus_remainder / 1000), 1, TOTLE_TOTLE_TOP);
	bonus_remainder = (bonus_remainder % 1000);
	totle_metric_digit_animate((bonus_remainder / 100),  2, TOTLE_TOTLE_TOP);
	bonus_remainder = (bonus_remainder % 100);
	totle_metric_digit_animate((bonus_remainder / 10),   3, TOTLE_TOTLE_TOP);
	bonus_remainder = (bonus_remainder % 10);
	totle_metric_digit_animate(bonus_remainder,          4, TOTLE_TOTLE_TOP);
	frame_delay(5);

#ifdef TH01_NATIVE_ORB_PHYSICS
 if(totle_show_automatic_result()) {
  score_bonus = 0;
  totle_free();
  return;
 }
#endif
	input_reset_sense();
	input_shot = true;
	input_ok = true;

	while((input_shot == true) && (input_ok == true)) {
		input_sense(false);
	}

	score_bonus = 0;
	totle_free();
}
