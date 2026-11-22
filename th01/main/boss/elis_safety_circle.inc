elis_starpattern_ret_t pattern_safety_circle_and_rain_from_top(void)
{
	enum {
		CIRCLE_FRAMES = 200,

		// A bit unfair, since the kill zone already starts in the transparent
		// parts of the non-moving player sprite.
		SAFETY_OFFSET_LEFT = BIGCIRCLE_RADIUS,

		// Slightly less unfair.
		SAFETY_OFFSET_RIGHT = (
			BIGCIRCLE_RADIUS - (PLAYER_W / 2) - (PLAYER_W / 4)
		),
	};

	static struct {
		int frame;
		screen_x_t target_left;
		unsigned char angle;

		screen_x_t center_x(void) { return (target_left + (PLAYER_W / 2)); };
		screen_y_t center_y(void) { return player_center_y(); };
	} circle;

	if(boss_phase_frame < 10) {
		circle.frame = 0;
	}

	ent_attack_render();

	if(boss_phase_frame < 60) {
		return SP_PATTERN;
	}
	if(boss_phase_frame == 60) {
		select_for_rank(pattern_state.interval, 4, 2, 2, 2);
		circle.angle = 0x00;
	}
	if(bigcircle_is_summon_frame(60) && (circle.frame == 0)) {
		if(boss_phase_frame == 60) {
			mdrv2_se_play(8);
			circle.target_left = player_left;
		}
		bigcircle_summon_update_and_render(circle, 0x02);
		if(bigcircle_summon_done(circle)) {
			bigcircle_sloppy_unput(circle);	// ZUN bloat: Position unchanged
			bigcircle_put(circle, V_WHITE);

			circle.frame = 1;
			ent_unput_and_put_both(ent_still_or_wave, 1, C_STILL);
		}
	} else if((circle.frame != 0) && (circle.frame < CIRCLE_FRAMES)) {
		circle.frame++;

		// We only blit the circle to VRAM page 0, so any unblitting call for
		// any overlapping sprite will "cut a hole" into the circle. So, um...
		// let's just re-blit it every 4 frames? :zunpet:
		if((circle.frame % 4) == 0) {
			bigcircle_sloppy_unput(circle); // ZUN bloat: Position unchanged
			bigcircle_put(circle, V_WHITE);
		}
		if(!player_invincible) {
			if(
				((circle.target_left + SAFETY_OFFSET_RIGHT) <= player_left) ||
				((circle.target_left - SAFETY_OFFSET_LEFT) >= player_left)
			) {
				delay(100);
				player_is_hit = true;
			}
		}
		if(player_is_hit == true) {
			circle.frame = CIRCLE_FRAMES;
		}
		if((boss_phase_frame % pattern_state.interval) == 0) {
			Pellets.add_group(
				(PLAYFIELD_LEFT + (irand() % (PLAYFIELD_W - PELLET_W))),
				(PLAYFIELD_TOP),
				(rank == RANK_LUNATIC) ? PG_1_RANDOM_WIDE : PG_1,
				to_sp(4.5f)
			);
		}
		if((circle.frame > (CIRCLE_FRAMES - 20)) && ((circle.frame % 4) == 0)) {
			form_fire_group(F_GIRL, PG_1_AIMED, 4.5f);
		}
	} else if(circle.frame < CIRCLE_FRAMES) {
		return SP_PATTERN;
	} else {
		if(circle.frame == CIRCLE_FRAMES) {
			bigcircle_sloppy_unput(circle);
		}
		circle.frame++;
		if((circle.frame % 8) == 0) {
			// ZUN quirk: Spawning pellets relative to the top-left corner of
			// the bat sprite rather than the girl one. Since this pattern only
			// ever runs after a bat transformation, the position isn't
			// *completely* random at least, just calculated a lot lower than
			// you would expect.
			Pellets.add_group(
				(ent_bat.cur_left + (GIRL_W / 2) - (PELLET_W / 2)),
				(ent_bat.cur_top  + (GIRL_H / 2) - (PELLET_H / 2)),
				PG_1_AIMED,
				to_sp(4.5f)
			);
		}
		if(circle.frame > (CIRCLE_FRAMES + 60)) {
			boss_phase_frame = 0;
			circle.frame = 0; // ZUN bloat: Gets reset at the beginning.
			circle.angle = 0x00; // ZUN bloat: Gets reset at the beginning.
			return SP_STAR_OF_DAVID;
		}
	}
	return SP_PATTERN;
}

