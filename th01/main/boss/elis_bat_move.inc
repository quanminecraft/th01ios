		if(form != F_GIRL) {
			ent_bat.locked_move_unput_and_put_8(
				bat_velocity_x, bat_velocity_y, (BAT_SPEED_DIVISOR - 1)
			);

			#define bat_cycle_frame (boss_phase_frame % BAT_CYCLE_FRAMES)
			if(bat_cycle_frame == (BAT_FRAMES_PER_CEL * 0)) {
				ent_bat.set_image(C_BAT + 0);
			} else if(bat_cycle_frame == (BAT_FRAMES_PER_CEL * 1)) {
				ent_bat.set_image(C_BAT + 1);
			} else if(bat_cycle_frame == (BAT_FRAMES_PER_CEL * 2)) {
				ent_bat.set_image(C_BAT + 2);
			}
			#undef bat_cycle_frame
		}
		