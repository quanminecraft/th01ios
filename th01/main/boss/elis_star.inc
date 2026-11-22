void near star_of_david_put(vc_t col)
{
	starcircle_line_put(-0x40, +0x16, col);
	starcircle_line_put(-0x40, +0x6A, col);
	starcircle_line_put(+0x16, +0x6A, col);
	starcircle_line_put(+0x40, -0x6A, col);
	starcircle_line_put(+0x40, -0x16, col);
	starcircle_line_put(-0x6A, -0x16, col);
}

inline void star_of_david_unput(void) {
	starcircle_line_unput(-0x40, +0x16);
	starcircle_line_unput(-0x40, +0x6A);
	starcircle_line_unput(+0x16, +0x6A);
	starcircle_line_unput(+0x40, -0x6A);
	starcircle_line_unput(+0x40, -0x16);
	starcircle_line_unput(-0x6A, -0x16);
}

// Renders a frame of the Star of David summon/flash animation in front of a
// danmaku pattern.
elis_starpattern_ret_t near star_of_david(void)
{
	static StarCircle circle;

	if(boss_phase_frame < 5) {
		circle.frame = 0;
		circle.angle = 0x00;
	}

	if(boss_phase_frame < 10) {
		return SP_STAR_OF_DAVID;
	}
	if(boss_phase_frame == 10) {
		ent_unput_and_put_both(ent_attack, 2, C_PREPARE);
		circle.angle = 0x00;
		circle.frame = 0;
	}
	if(bigcircle_is_summon_frame(10) && (circle.frame == 0)) {
		if(bigcircle_summon(circle, 10, 0x02)) {
			circle.frame = 1;
			bigcircle_sloppy_unput(circle);	// ZUN bloat: Position unchanged
			bigcircle_put(circle, V_WHITE);
		}
	} else if(bigcircle_summon_done(circle)) {
		circle.frame++;
		if(circle.frame == 10) {
			star_of_david_put(V_WHITE);
		}
		if((circle.frame > 20) && ((circle.frame % 4) == 0)) {
			star_of_david_put(COL_FX);
			bigcircle_put(circle, COL_FX);
		} else if((circle.frame > 20) && ((circle.frame % 4) == 2)) {
			star_of_david_put(V_WHITE);
			bigcircle_put(circle, V_WHITE);
		}
		if(circle.frame > 60) {
			star_of_david_unput();
			bigcircle_sloppy_unput(circle);
			boss_phase_frame = 0;
			circle.angle = 0x00;
			circle.frame = 0;
			return SP_PATTERN;
		}
	}
	return SP_STAR_OF_DAVID;
}

