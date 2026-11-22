void pattern_hop_and_fire_chase_pellets(bool16 do_not_initialize = true)
{
	enum {
		HOP_DISTANCE = (PLAYFIELD_W / 5),
		SPREADIN_INTERVAL = 4,
		SPREADIN_FRAMES = 20,

		KEYFRAME_HOP = 1,
		KEYFRAME_TELEPORT = 4,
		KEYFRAME_HITBOX_ACTIVE = 12,
		KEYFRAME_SPREADIN_DONE = (KEYFRAME_SPREADIN_START + SPREADIN_FRAMES),
	};

	static uint8_t hop;
	static x_direction_t direction;
	int i;
	int pellet_count;
	unsigned char angle;

	// Most of this could have been statically initialized, and even the random
	// direction wouldn't have required this separate parameter.
	if(!do_not_initialize) {
		hop = -1;
		spreadin_interval = SPREADIN_INTERVAL;
		direction = static_cast<x_direction_t>(irand() % 2);
		spreadin_speed = ((MIMA_H / 2) / (SPREADIN_FRAMES / SPREADIN_INTERVAL));
		return;
	}

	if(hop == static_cast<uint8_t>(-1)) {
		mima_unput_both();
		meteor_active = false;
		hop = 0;
		boss_phase_frame = KEYFRAME_HOP;
	}
	if(hop == 0) {
		ent_still.hitbox_orb_inactive = true;
		mima_vertical_sprite_transition_broken();
	} else {
		if(boss_phase_frame == 4) {
			ent_still.pos_cur_set(
				((direction == X_RIGHT)
					? (PLAYFIELD_LEFT + ((hop - 1) * HOP_DISTANCE))
					: (PLAYFIELD_RIGHT - (hop * HOP_DISTANCE))
				),
				(BASE_TOP - (PLAYFIELD_H / 14)) // Yup, not centered!
			);
			mima_bg_snap();
		}
		if(boss_phase_frame < KEYFRAME_HITBOX_ACTIVE) {
			ent_still.hitbox_orb_inactive = true;
		} else {
			ent_still.hitbox_orb_inactive = false;
		}
		phase_spreadin(ent_still.cur_left, ent_still.cur_top);
	}

	// Spread-in animation done?
	if(boss_phase_frame != 0) {
		return;
	}

	if(hop != 0) {
		if(hop != 4) {
			mima_unput_both();
		}

		select_subpixel_for_rank(
			pattern_state.speed, 1.875f, 2.1875f, 2.5f, 2.8125f
		);
		select_for_rank(pellet_count, 5, 8, 10, 12);

		for(
			(i = 0, angle = 0x00);
			i < pellet_count;
			(i++, angle += (0x100 / pellet_count))
		) {
			Pellets.add_single(
				ent_still.cur_center_x(),
				ent_still.cur_center_y(),
				angle,
				pattern_state.speed,
				PM_CHASE,
				pattern_state.speed
			);
		}
	}
	if(hop >= 4) {
		// MODDERS: Same as mima_put_still_both().
		page_access(1);	ent_still.put_8(0);
		page_access(0);	ent_still.put_8(0);

		z_palette_set_all_show(stage_palette);

		// Prepare a potential next run of this pattern
		hop = -1;
		direction = static_cast<x_direction_t>(X_LEFT - direction);

		meteor_active = true;
		boss_phase_frame = 0;
	} else {
		// Subtract #444 from every palette color for every new hop. Therefore,
		// you would expect the screen to gradually darken with every
		// successive hop, eventually reaching #000 by the last one… which is
		// not what you see in-game, thanks to boss_hit_update_and_render()
		// periodically resetting [z_Palettes] to the [boss_palette]. Hard to
		// say what ZUN actually intended here – 30 frames of being completely
		// blind to 3 groups of chase pellets isn't all *too* bad, even on
		// Lunatic. (Then again, at maximum pellet speed...)
		for(i = 0; i < COLOR_COUNT; i++) {
			z_palette_set_show(
				i,
				(z_Palettes[i].c.r - 0x4),
				(z_Palettes[i].c.g - 0x4),
				(z_Palettes[i].c.b - 0x4)
			);
		}
		hop++;
		boss_phase_frame = KEYFRAME_HOP;
	}
}

