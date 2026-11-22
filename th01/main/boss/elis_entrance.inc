		enum {
			SPHERE_COUNT = 2,
			TRAIL_COUNT = 3,
		};

		// Trail (i + 1) = position of trail (i) in the previous frame. The
		// head coordinates are stored separately, and trail (TRAIL_COUNT - 1)
		// is never rendered.
		struct {
			screen_y_t top[SPHERE_COUNT][TRAIL_COUNT];
			screen_x_t left[SPHERE_COUNT][TRAIL_COUNT];
		} trails;
#ifdef TH01_NATIVE_ORB_PHYSICS
        // Original stack garbage is not a portable initial trail position.
        for(int i=0;i<SPHERE_COUNT;++i)for(int j=0;j<TRAIL_COUNT;++j){trails.left[i][j]=RES_X;trails.top[i][j]=RES_Y;}
#endif

		#define sphere_unput_and_put_head(sphere_i, head_left, head_top) { \
			sphere_unput(trails.left[sphere_i][0], trails.top[sphere_i][0]); \
			sphere_put(head_left, head_top, C_SPHERE_LARGE); \
		}

		#define sphere_unput_and_put_trail(sphere_i, trail_i) { \
			sphere_unput( \
				trails.left[sphere_i][trail_i + 1], \
				trails.top[sphere_i][trail_i + 1] \
			); \
			sphere_put( \
				trails.left[sphere_i][trail_i], \
				trails.top[sphere_i][trail_i], \
				(trail_i == 0) ? C_SPHERE_MEDIUM : C_SPHERE_SMALL \
			); \
		}

		#define sphere_trails_forward_copy(sphere_i, head_left, head_top) { \
			trails.left[sphere_i][2] = trails.left[sphere_i][1]; \
			trails.top[sphere_i][2] = trails.top[sphere_i][1]; \
			trails.left[sphere_i][1] = trails.left[sphere_i][0]; \
			trails.top[sphere_i][1] = trails.top[sphere_i][0]; \
			trails.left[sphere_i][0] = head_left; \
			trails.top[sphere_i][0] = head_top; \
		}

		girl_bg_snap(1);

		// Trailing rotation
		// -----------------

		#define entrance_tick hit.invincibility_frame

		angle = 0x00;
		entrance_tick = 0;
		while(1) {
			entrance_tick++;
			for(int i = 0; i < SPHERE_COUNT; i++) {
				head_left = polar_x(
					(BASE_CENTER_X - (SPHERE_W / 2)),
					((GIRL_H * 2) / 2),
					(angle + (i * (0x100 / SPHERE_COUNT))
				));
				head_top = polar_y(
					// Yup, slightly off-center.
					((BASE_CENTER_Y - (SPHERE_H / 2)) - 14),
					((GIRL_H * 2) / 2),
					(angle + (i * (0x100 / SPHERE_COUNT)))
				);

				// ZUN landmine: Reads uninitialized stack memory in frame 0.
				// Thankfully doesn't end up hitting the player or orb sprite
				// in the original binary.
				sphere_unput_and_put_head(i, head_left, head_top);

				// ZUN landmine: Both of these calls read uninitialized stack
				// memory during frames 2 and 3, respectively. Invisible as
				// well.
				if(entrance_tick > 1) {
					sphere_unput_and_put_trail(i, 0);
					if(entrance_tick > 2) {
						sphere_unput_and_put_trail(i, 1);
					}
				}
				sphere_trails_forward_copy(i, head_left, head_top);
			}
			if(entrance_tick >= 120) {
				break;
			}
			angle += (0x300 / 32);
			frame_delay(((120 - entrance_tick) / 12) + 1);
		};

		#undef entrance_tick
		// -----------------

		// Spheres move offscreen / wave-in effect
		// ---------------------------------------

		enum {
			MOVE_INTERVAL = 4,
			MOVE_VELOCITY = (PLAYFIELD_W / 40),

			WAVE_INTERVAL = 4,
			KEYFRAME_WAVE = 20,
			KEYFRAME_WAVE_DONE = 100,
			KEYFRAME_SLIGHT_RIPPLE = 128,
			KEYFRAME_SLIGHT_RIPPLE_DONE = 130,
			KEYFRAME_ENTRANCE_DONE = 140,
		};

		#define entrance_frame hit.invincibility_frame

		entrance_frame = 0;
		trails_offscreen = false;
		while(1) {
			entrance_frame++;
			if(!trails_offscreen && ((entrance_frame % MOVE_INTERVAL) == 0)) {
				for(int i = 0; i < SPHERE_COUNT; i++) {
					head_left = (
						(trails.left[i][0] + (MOVE_VELOCITY * MOVE_INTERVAL)) -
						(i * (MOVE_VELOCITY * MOVE_INTERVAL * 2))
					);
					head_top = trails.top[i][0];

					sphere_unput_and_put_head(i, head_left, head_top);
					sphere_unput_and_put_trail(i, 0);
					sphere_unput_and_put_trail(i, 1);

					if(
						(trails.left[0][2] > RES_X) &&
						(trails.left[1][2] < (0 - SPHERE_W))
					) {
						trails_offscreen = true;
					}

					sphere_trails_forward_copy(i, head_left, head_top);
				}
			}
			if(
				(entrance_frame > KEYFRAME_WAVE) &&
				(entrance_frame < KEYFRAME_WAVE_DONE)
			) {
				if((entrance_frame % WAVE_INTERVAL) == 0) {
					if(entrance_frame != (KEYFRAME_WAVE + WAVE_INTERVAL)) {
						// Amplitude should technically be
						//
						// 	((KEYFRAME_WAVE_DONE - WAVE_INTERVAL) - frame)
						//
						// here (not +), matching the blitting function below.
						// Good that this function is so sloppy with all its
						// word alignment that it actually makes no difference.
						ent_still_or_wave.wave_sloppy_unput(
							((entrance_frame - WAVE_INTERVAL) + 2),
							(
								(KEYFRAME_WAVE_DONE + WAVE_INTERVAL) -
								entrance_frame
							),
							((entrance_frame - WAVE_INTERVAL) * 6)
						);
					}
					ent_still_or_wave.wave_put(
						C_STILL,
						(entrance_frame + 2),
						(KEYFRAME_WAVE_DONE - entrance_frame),
						(entrance_frame * 6)
					);
				}
			} else if(entrance_frame == KEYFRAME_WAVE_DONE) {
				// Phase should technically be (entrance_frame * 6), but...
				// yeah, see above. No difference here either.
				ent_still_or_wave.wave_sloppy_unput(
					(entrance_frame + 2),
					(KEYFRAME_WAVE_DONE - entrance_frame),
					entrance_frame
				);
				// Unnecessary unblitting...
				ent_still_or_wave.unlock_unput_put_image_lock_8(C_STILL);
			} else if(entrance_frame == KEYFRAME_SLIGHT_RIPPLE) {
				girl_bg_put(1);
				ent_still_or_wave.wave_put(C_STILL, 3, 8, 64);
			} else if(entrance_frame == KEYFRAME_SLIGHT_RIPPLE_DONE) {
				ent_still_or_wave.wave_sloppy_unput(3, 8, 64);
				// Unnecessary unblitting...
				ent_still_or_wave.unlock_unput_put_image_lock_8(C_STILL);
			} else if(entrance_frame > KEYFRAME_ENTRANCE_DONE) {
				break;
			}
			frame_delay(1);
		}

		#undef entrance_frame
		// ---------------------------------------

		#undef sphere_trails_forward_copy
		#undef sphere_unput_and_put_trail
		#undef sphere_unput_and_put_head

		boss_phase_frame = 0;
		hit.invincibility_frame = 0;
		boss_phase = 1;
		ent_still_or_wave.hitbox_orb_inactive = false;

		// Necessary, since the entire entrance animation was only played on
		// VRAM page 0...
		page_access(1); ent_still_or_wave.unlock_put_lock_8();

		// ... which makes this blitting call redundant, though.
		page_access(0); ent_still_or_wave.unlock_put_lock_8();

		phase.teleport_done = false;
		phase.cur.pattern = 1;
		initial_hp_rendered = false;
	