bool16 wave_teleport(screen_x_t target_left, screen_y_t target_top)
{
	ent_attack.pos_cur_set_to(ent_still_or_wave);

	// Wave sprite
	if(boss_phase_frame == 20) {
		page_access(1);
		girl_bg_put(1);
		page_access(0);
		ent_still_or_wave.unlock_put_image_lock_8(C_WAVE_1);
		ent_still_or_wave.hitbox_orb_inactive = true;
	} else if(boss_phase_frame == 28) {
		girl_bg_put(1);
		ent_still_or_wave.unlock_put_image_lock_8(C_WAVE_2);
		ent_still_or_wave.hitbox_orb_inactive = true;
	} else if(boss_phase_frame == 36) {
		ent_still_or_wave.hitbox_orb_inactive = true;
		girl_bg_put(1);
		ent_still_or_wave.unlock_put_image_lock_8(C_WAVE_3);
	} else if(boss_phase_frame == 44) {
		ent_still_or_wave.hitbox_orb_inactive = true;
		egc_copy_rect_1_to_0_16(
			ent_still_or_wave.cur_left,
			ent_still_or_wave.cur_top,
			GIRL_W,
			GIRL_H
		);
	} else if(boss_phase_frame == 52) {
		ent_still_or_wave.hitbox_orb_inactive = true;
		ent_still_or_wave.pos_cur_set(target_left, target_top);
		girl_bg_snap(1);
		girl_bg_put(1); // unnecessary
		ent_still_or_wave.unlock_put_image_lock_8(C_WAVE_3);
	} else if(boss_phase_frame == 60) {
		ent_still_or_wave.hitbox_orb_inactive = true;
		girl_bg_put(1);
		ent_still_or_wave.unlock_put_image_lock_8(C_WAVE_2);
	} else if(boss_phase_frame == 68) {
		ent_still_or_wave.hitbox_orb_inactive = true;
		girl_bg_put(1);
		ent_still_or_wave.unlock_put_image_lock_8(C_WAVE_1);
	} else if(boss_phase_frame == 76) {
		ent_still_or_wave.hitbox_orb_inactive = false;
		page_access(1);
		ent_still_or_wave.unlock_put_image_lock_8(C_STILL);
		page_access(0);
		ent_still_or_wave.unlock_put_image_lock_8(C_STILL);
	} else if(boss_phase_frame > 80) {
		boss_phase_frame = 0;
		return true;
	}

	// Stars
	if((boss_phase_frame % 4) != 0) {
		return false;
	}

	static EntitiesTopleft<5> stars;

	for(int i = 0; i < stars.count(); i++) {
		if(boss_phase_frame > 4) {
			egc_copy_rect_1_to_0_16_word_w(stars.left[i], stars.top[i], 8, 8);
		}
		if((boss_phase_frame < 40) || (boss_phase_frame > 52)) {
			stars.left[i] = surround_random_left(ent_still_or_wave);
			stars.top[i] = surround_random_top(ent_still_or_wave);
		} else {
			stars.left[i] = (stars.left[i] + (
				(surround_random_left(ent_still_or_wave) - stars.left[i]) / 3
			));
			stars.top[i] = (stars.top[i] + (
				(surround_random_top(ent_still_or_wave) - stars.top[i]) / 3
			));
		}
		if(boss_phase_frame < 68) {
			shape8x8_star_put(stars.left[i], stars.top[i], 2);
		}
	}
	return false;
}

