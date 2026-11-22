elis_phase_5_subphase_t bat_fly_random(pixel_t &velocity_x, pixel_t &velocity_y)
{
	enum {
		SPEED = 2,
	};

	static screen_x_t target_left; // should be local
	static screen_y_t target_top; // should be local
	static int frames_until_target;

	if(boss_phase_frame < BAT_SPEED_DIVISOR) {
		velocity_x = 0;
		velocity_y = 0;
		return P5_PATTERN;
	}
	if(boss_phase_frame == BAT_SPEED_DIVISOR) {
		do {
			target_left = elis_playfield_random_left();
			target_top = (elis_playfield_random_top() + ((GIRL_H - BAT_H) / 2));
		} while(
			(abs(ent_bat.cur_left - target_left) < (PLAYFIELD_W / 5)) &&
			(abs(ent_bat.cur_top - target_top) < ((PLAYFIELD_H * 4) / 21))
		);
		vector2_between(
			ent_bat.cur_left,
			ent_bat.cur_top,
			target_left,
			target_top,
			velocity_x,
			velocity_y,
			SPEED
		);
		frames_until_target = (abs(target_left - ent_bat.cur_left) / SPEED);
	}

	// What is something like "gravity" doing in a bat movement function?
	// Quite the interesting interpretation as well, even if it wasn't always
	// zero. MODDERS: Remove.
	velocity_y += (((irand() % 5) - 2) / 10);

	if((boss_phase_frame / BAT_SPEED_DIVISOR) >= frames_until_target) {
		velocity_x = 0;
		velocity_y = 0;
		boss_phase_frame = 0;
		return P5_TRANSFORM;
	}
	return P5_PATTERN;
}

