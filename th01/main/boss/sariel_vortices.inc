static const subpixel_t VORTEX_PELLET_SPEED = TO_SP(7);

#define vortex_pellet_left(vortex_left) \
	((vortex_left) + ((VORTEX_W / 2) - (PELLET_W / 2)))

#define vortex_pellet_top(vortex_top) \
	((vortex_top) + ((VORTEX_H / 2) - (PELLET_H / 2)))

inline screen_y_t vortex_pellet_bottom(screen_y_t vortex_bottom) {
	return (vortex_bottom - ((VORTEX_H / 2) - (PELLET_H / 2)));
}

// Shouldn't really take [angle] as a parameter...
void near vortex_fire_3_spread(
	const screen_x_t left[VORTEX_COUNT],
	const vram_y_t top[VORTEX_COUNT],
	int i,
	unsigned char angle
)
{
	Pellets.add_single(
		vortex_pellet_left(left[i]),
		vortex_pellet_top(top[i]),
		0x40,
		VORTEX_PELLET_SPEED
	);
	angle = iatan2(
		(vortex_pellet_bottom(PLAYFIELD_BOTTOM) - top[i]),
		(left[1 - i] - left[i])
	);
	Pellets.add_single(
		vortex_pellet_left(left[i]),
		vortex_pellet_top(top[i]),
		angle,
		VORTEX_PELLET_SPEED
	);
	Pellets.add_single(
		vortex_pellet_left(left[i]),
		vortex_pellet_top(top[i]),
		(0x80 - angle),
		VORTEX_PELLET_SPEED
	);
}

void near pattern_vortices(void)
{
	static bool16 wand_raise_animation_done = false;
	static EntitiesTopleft<VORTEX_COUNT> cur;
	static EntitiesTopleft<VORTEX_COUNT> prev;
	static bool16 dir_first; // x_direction_t
	static bool16 dir_second; // x_direction_t

#ifdef TH01_NATIVE_ORB_PHYSICS
	if(native_actor_reset) { wand_raise_animation_done = false; cur = {}; prev = {}; dir_first = dir_second = 0; return; }
#endif

	#define vortex_unput_and_put_8(i) { \
		grc_sloppy_unput(prev.left[i], prev.top[i]); \
		vortex_or_debris_put_8( \
			cur.left[i], \
			cur.top[i], \
			(C_VORTEX + (boss_phase_frame % VORTEX_CELS)) \
		); \
	}

	#define vortex_fire_down(i) { \
		Pellets.add_group( \
			vortex_pellet_left(cur.left[i]), \
			vortex_pellet_top(cur.top[i]), \
			PG_1, \
			VORTEX_PELLET_SPEED \
		); \
	}

	#define vortex_unput_put_3_spread(i) { \
		vortex_unput_and_put_8(i); \
		if((boss_phase_frame % 4) == 0) { \
			vortex_fire_3_spread(cur.left, cur.top, i, angle); \
		} \
	}

#ifdef TH01_NATIVE_ORB_PHYSICS
	unsigned char angle = 0; // Input is overwritten inside vortex_fire_3_spread.
#else
	unsigned char angle;
#endif

	if(wand_raise_animation_done == false) {
		wand_raise_animation_done = wand_render_raise_both();
	}

	if(boss_phase_frame < 50) {
		return;
	} else if(boss_phase_frame == 50) {
		select_for_rank(pattern_state.frame, 140, 145, 150, 155);
	}
	if(boss_phase_frame < 100) {
		for(int i = 0; i < VORTEX_COUNT; i++) {
			cur.left[i] = (i * (PLAYFIELD_RIGHT - VORTEX_W));
			cur.top[i] = (PLAYFIELD_TOP + playfield_fraction_y(17 / 42.0f));

			vortex_unput_and_put_8(i);
			if((boss_phase_frame % 4) == 0) {
				vortex_fire_down(i);
			}
			prev.left[i] = cur.left[i];
			prev.top[i] = cur.top[i];
		}
	} else if(boss_phase_frame < pattern_state.frame) {
		for(int i = 0; i < VORTEX_COUNT; i++) {
			cur.left[i] += (i == 0) ? 5 : -5;
			cur.top[i] -= 2;

			vortex_unput_and_put_8(i);
			if((boss_phase_frame % 4) == 0) {
				vortex_fire_down(i);
				angle = iatan2(
					vortex_pellet_top(PLAYFIELD_BOTTOM - cur.top[i]),
					vortex_pellet_left(cur.left[i] +
						(i * ((PLAYFIELD_W / 8) * 2)) - (PLAYFIELD_W / 8) -
					cur.left[i])
				);
				Pellets.add_single(
					vortex_pellet_left(cur.left[i]),
					vortex_pellet_top(cur.top[i]),
					angle,
					VORTEX_PELLET_SPEED
				);
			}
			prev.left[i] = cur.left[i];
			prev.top[i] = cur.top[i];
		}
	} else if(boss_phase_frame < 200) {
		for(int i = 0; i < VORTEX_COUNT; i++) {
			vortex_unput_put_3_spread(i);
		}
	} else if(boss_phase_frame < 240) {
		if(boss_phase_frame == 200) {
#ifdef TH01_NATIVE_ORB_PHYSICS
			dir_first = (irand() % 2);
#else
			static_cast<int>(dir_first) = (irand() % 2);
#endif
			if(dir_first == X_LEFT) {
				wand_lower_both();
			}
		}
		for(int i = 0; i < VORTEX_COUNT; i++) {
			cur.left[i] += (2 - (dir_first * 4));
			vortex_unput_put_3_spread(i);
			prev.left[i] = cur.left[i];
		}
	} else if(boss_phase_frame < 320) {
		if(boss_phase_frame == 240) {
#ifdef TH01_NATIVE_ORB_PHYSICS
			dir_second = (irand() % 2);
#else
			static_cast<int>(dir_second) = (irand() % 2);
#endif
			if((dir_second == X_RIGHT) && (dir_second == dir_first)) {
				wand_lower_both();
			} else if((dir_second == X_LEFT) && (dir_second == dir_first)) {
				wand_raise_animation_done = false;
			}
		}
		for(int i = 0; i < VORTEX_COUNT; i++) {
			if(dir_second != dir_first) {
				cur.left[i] -= (2 - (dir_second * 4));
			} else {
				cur.left[i] -= (4 - (dir_second * 8));
			}
			vortex_unput_put_3_spread(i);
			prev.left[i] = cur.left[i];
		}
	} else if(boss_phase_frame > 300) {
		for(int i = 0; i < VORTEX_COUNT; i++) {
			grc_sloppy_unput(cur.left[i], cur.top[i]);
		}
		wand_lower_both();
		boss_phase_frame = 0;
		wand_raise_animation_done = false;
	}

	#undef vortex_unput_put_3_spread
	#undef vortex_fire_down
	#undef vortex_unput_and_put_8
}
