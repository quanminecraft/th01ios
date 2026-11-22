void near pattern_symmetric_birds_from_bottom(void)
{
	enum {
		VELOCITY_X = 2,
	};

	static SymmetricSpawnraysWithDebris<
		PLAYFIELD_CENTER_X, FACE_CENTER_Y, (DEBRIS_W / 4)
	> rays;
	static point_t velocity;
	static vortex_or_debris_cel_t debris_cel;

	double target_x; // double?!

	if(boss_phase_frame < 50) {
		return;
	}
	if(boss_phase_frame == 50) {
		select_for_rank(pattern_state.unknown, 6, 7, 8, 9);
		rays.target_l_x = VELOCITY_X;
		rays.target_y = (PLAYFIELD_BOTTOM - 1);
		velocity.x = VELOCITY_X;
		velocity.y = -2; // could have been a constant
		debris_cel = C_DEBRIS;
		return;
	} else if(boss_phase_frame < 100) {
		if((boss_phase_frame % 2) == 0) {
			rays.unput_and_put(X_LEFT, debris_cel, VELOCITY_X);
		}
		if((boss_phase_frame % 2) == 1) {
			rays.unput_and_put(X_RIGHT, debris_cel, VELOCITY_X);
			debris_cel = debris_cel_next(debris_cel);
		}
		rays.target_l_x += VELOCITY_X;
		if((boss_phase_frame % 10) == 0) {
			mdrv2_se_play(6);

			target_x = (((boss_phase_frame - 60) * 10) + rays.target_l_x);
			vector2_between(
				rays.target_x(X_LEFT), (rays.target_y - BIRD_H),
				target_x, 0,
				velocity.x, velocity.y,
				4
			);
			Birds.spawn(
				rays.target_x(X_LEFT),
				(rays.target_y - BIRD_H),
				velocity.x,
				velocity.y
			);
			Birds.spawn(
				rays.target_x(X_RIGHT),
				(rays.target_y - BIRD_H),
				-velocity.x,
				velocity.y
			);
		}
		if((boss_phase_frame % 6) == 0) {
			Birds.fire(BPG_RANDOM_RAIN);
		}
		return;
	} else if(boss_phase_frame == 100) {
		rays.ray_unput_right(VELOCITY_X);
		return;
	} else if((boss_phase_frame % 10) == 0) {
		Birds.fire(BPG_RANDOM_RAIN);
	}
	if(boss_phase_frame >= 300) {
		boss_phase_frame = 0;
	}
}
