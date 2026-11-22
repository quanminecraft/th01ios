int pattern_curved_5_stack_rings(void)
{
	#define fire_ring(i, angle_offset, speed) { \
		for(int i = 0; i < pattern_state.ring; i++) { \
			Pellets.add_single( \
				(form_center_x(F_GIRL) - (PELLET_W / 2)), \
				(form_center_y(F_GIRL) - (PELLET_H / 2)), \
				(((0x100 / pattern_state.ring) * i) + angle_offset), \
				to_sp(speed) \
			); \
		} \
	}

	ent_attack_render();
	if(boss_phase_frame == 10) {
		select_for_rank(pattern_state.ring, 14, 16, 18, 20);
		fire_ring(i, 0x00, 3.0f);
	} else if(boss_phase_frame == 16) {
		fire_ring(i, 0x02, 3.375f);
	} else if(boss_phase_frame == 24) {
		fire_ring(i, 0x04, 4.0f);
	} else if(boss_phase_frame == 32) {
		fire_ring(i, 0x06, 4.5f);
	} else if(boss_phase_frame == 40) {
		fire_ring(i, 0x08, 5.0f);
	} else if(boss_phase_frame > 60) {
		boss_phase_frame = 0;

		// ZUN bug: A completely unnecessary unblitting call that doesn't even
		// switch back to VRAM page 0. At first, this might look like it has no
		// effect since Elis runs a wave_teleport() animation afterwards. But
		// if any player shot, bomb, or Orb sprites overlap Elis between the
		// end of this pattern and the start of the teleport animation, their
		// corresponding unblitting calls will rip holes into the Elis sprite.
		page_access(1);
		girl_bg_put(2);

		return CHOOSE_NEW;
	}
	return 1;

	#undef fire_ring
}

