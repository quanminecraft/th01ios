void pattern_two_homing_snakes_and_semicircle_spreads(void)
{
	static Snakes<2> snakes;

	int i;
	int j;
	screen_x_t pellet_left;
	screen_y_t pellet_top;
	unsigned char angle;

	if(boss_phase_frame == 10) {
		face_expression_set_and_put(FE_GLARE);
	}
	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		snakes.target_locked[0] = false;
		snakes.target_locked[1] = false;
		snakes_spawn_and_wobbly_aim(snakes, 0, CUP_CENTER_X, CUP_TOP, i, angle);
		snakes.left[1][0] = -PIXEL_NONE;
		mdrv2_se_play(12);
		return;
	}
	if(boss_phase_frame < 500) {
		snakes_unput_update_render(i, j, angle)
		if(boss_phase_frame == 150) {
			snakes_spawn_and_wobbly_aim(
				snakes, 1, CUP_CENTER_X, CUP_TOP, i, angle
			);
			mdrv2_se_play(12);
		}

		if(
			(boss_phase_frame > (240 - (rank * 40))) &&
			((boss_phase_frame % 40) == 0)
		) {
			enum {
				SPREAD = 10,
			};
			pixel_t velocity_x;
			pixel_t velocity_y;
			Subpixel speed;

			angle = (irand() % (0x80 / SPREAD));
			pellet_left =
				((boss_phase_frame % 120) ==  0) ? SWORD_CENTER_X :
				((boss_phase_frame % 120) == 40) ? EYE_CENTER_X :
				/*boss_phase_frame % 120  == 80 */ CUP_CENTER_X;
			pellet_top =
				((boss_phase_frame % 120) ==  0) ? SWORD_CENTER_Y :
				((boss_phase_frame % 120) == 40) ? EYE_BOTTOM :
				/*boss_phase_frame % 120  == 80 */ CUP_TOP;

			for(i = 0; i < SPREAD; i++) {
				speed.v = (to_sp(2.5f) + (
					((i % 4) == 0) ? to_sp(0.0f) :
					((i % 4) == 1) ? to_sp(1.0f) :
					((i % 4) == 2) ? to_sp(0.0f) :
					/*i % 4  == 3 */ to_sp(2.0f)
				));

				// That result is never used?
				vector2(velocity_x, velocity_y, speed, angle);

				Pellets.add_single(
					pellet_left, pellet_top, (0x80 - angle), speed
				);
				Pellets.add_single(pellet_left, pellet_top, angle, speed);
				angle += (0x80 / SPREAD);
			}
			mdrv2_se_play(7);
		};
	} else {
		snakes_unput_all(snakes, i, j);
		boss_phase_frame = 0;
	}
}

