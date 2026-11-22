
		// Downwards lasers from every eye, in a symmetric sequence from the
		// left and right edges of the playfield towards the center

		hud_hp_increment_render(
			phase.initial_hp_rendered, boss_hp, boss_phase_frame
		);

		boss_phase_frame++;

		eye_west.locked_unput_and_put_8();
		eye_east.locked_unput_and_put_8();
		eye_southwest.locked_unput_and_put_8();
		eye_southeast.locked_unput_and_put_8();
		eye_north.locked_unput_and_put_8();

		phase_0_downwards_lasers();

		if((boss_phase_frame % 40) == 0) {
			// Target color: (13, 13, 5) ⇒ #DD5
			z_Palettes[COL_YOKOSHIMA].c.r++; // ZUN bloat
			z_Palettes[COL_YOKOSHIMA].c.g++; // ZUN bloat
			stage_palette[COL_YOKOSHIMA].c.r++;
			stage_palette[COL_YOKOSHIMA].c.g++;
			z_palette_set_all_show(stage_palette);
		}
		if(boss_phase_frame == KEYFRAME_LATERAL_OPENING) {
			phase_0_eyes_open(KEYFRAME_LATERAL_OPENING);

			// ZUN bloat: Without a call to hit.update_and_render(), any hitbox
			// manipulation in this phase is doubly redundant...
			eye_west.hitbox_orb_activate();
			eye_east.hitbox_orb_activate();
		} else if(boss_phase_frame == KEYFRAME_SOUTH_OPENING) {
			phase_0_eyes_open(KEYFRAME_SOUTH_OPENING);
			eye_southwest.hitbox_orb_activate();
			eye_southeast.hitbox_orb_activate();
		} else if(boss_phase_frame == KEYFRAME_NORTH_OPENING) {
			phase_0_eyes_open(KEYFRAME_NORTH_OPENING);
			eye_north.hitbox_orb_activate();
		} else if(boss_phase_frame == KEYFRAME_LATERAL_OPEN) {
			phase_0_eyes_open(KEYFRAME_LATERAL_OPEN);
		} else if(boss_phase_frame == KEYFRAME_SOUTH_OPEN) {
			phase_0_eyes_open(KEYFRAME_SOUTH_OPEN);
		} else if(boss_phase_frame == KEYFRAME_NORTH_OPEN) {
			phase_0_eyes_open(KEYFRAME_NORTH_OPEN);
		} else if(boss_phase_frame == KEYFRAME_CLOSING) {
			phase_0_eyes_close(KEYFRAME_CLOSING);
			eye_west.hitbox_orb_deactivate();
			eye_east.hitbox_orb_deactivate();
			eye_southwest.hitbox_orb_deactivate();
			eye_southeast.hitbox_orb_deactivate();
			eye_north.hitbox_orb_deactivate();

			// ZUN quirk: Did you mean ">= RANK_HARD"? Because...
			if(rank == RANK_HARD) {
				phase_0_fire(eye_north);
				phase_0_fire(eye_southeast);
				phase_0_fire(eye_southwest);

				// ... this condition can now never be true, making the Lunatic
				// version of this subpattern effectively unused.
				if(rank == RANK_LUNATIC) {
					phase_0_fire(eye_west);
					phase_0_fire(eye_east);
				}
			}
		} else if(boss_phase_frame == KEYFRAME_CLOSED) {
			phase_0_eyes_close(KEYFRAME_CLOSED);

			if(rank == RANK_LUNATIC) {
				phase_0_fire(eye_north);
				phase_0_fire(eye_southeast);
				phase_0_fire(eye_southwest);
				phase_0_fire(eye_west);
				phase_0_fire(eye_east);
			}
		} else if(boss_phase_frame == KEYFRAME_HIDDEN) {
			phase_0_eyes_close(KEYFRAME_HIDDEN);
		} else if(boss_phase_frame == KEYFRAME_LATERAL_LASER_DONE) {
			eye_west.set_image(C_CLOSED);
			eye_east.set_image(C_CLOSED);

			eye_west.downwards_laser_unput();
			eye_east.downwards_laser_unput();
		} else if(boss_phase_frame == KEYFRAME_SOUTH_LASER_DONE) {
			eye_west.set_image(C_HALFOPEN);
			eye_east.set_image(C_HALFOPEN);

			eye_west.hitbox_orb_activate();
			eye_east.hitbox_orb_activate();

			eye_southwest.downwards_laser_unput();
			eye_southeast.downwards_laser_unput();
		} else if(boss_phase_frame > KEYFRAME_NORTH_LASER_DONE) {
			boss_phase = 1;
			hit.invincible = false;
			boss_phase_frame = 0;
			z_palette_set_all_show(stage_palette);
			boss_palette_snap();
			phase.initial_hp_rendered = false; // ZUN bloat

			// ZUN bloat: These won't be visible at all, since we get here on
			// the second frame with the entity lock active. On the next frame,
			// these two eyes will immediately adjust themselves to the
			// player's tracked position.
			eye_west.set_image(C_LEFT);
			eye_east.set_image(C_DOWN);

			pattern_interval = select_for_rank(350, 300, 200, 130);

			eye_north.downwards_laser_unput();
		}
	