void near pattern_birds_on_ellipse_arc(void)
{
	static bool wand_raise_animation_done = false;
	static bird_pellet_group_t pellet_group = BPG_AIMED;
	static int eggs_alive = 0;
	static EntitiesTopleft<BIRD_COUNT> egg;
	static Subpixel spawner_left;
	static Subpixel spawner_top;
	static Subpixel spawner_velocity_y;
	static Subpixel spawner_velocity_x;

#ifdef TH01_NATIVE_ORB_PHYSICS
	if(native_actor_reset) { wand_raise_animation_done = false; eggs_alive = 0; egg = {}; return; }
#endif

	point_t velocity;

	if(boss_phase_frame < 40) {
		return;
	}

	if(wand_raise_animation_done == false) {
		wand_raise_animation_done = wand_render_raise_both();
	}
	if(boss_phase_frame < 50) {
		return;
	} else if(boss_phase_frame == 50) {
		eggs_alive = 0;
		spawner_left.v = to_sp(WAND_EMIT_LEFT);
		spawner_top.v = to_sp(WAND_EMIT_TOP);
		spawner_velocity_x.v = TO_SP(4 - ((irand() % 2) * 8));
		spawner_velocity_y.v = TO_SP(2 - ((irand() % 2) * 4));
		select_for_rank(pattern_state.interval, 20, 15, 10, 8);
		mdrv2_se_play(8);
	} else if(boss_phase_frame < 200) {
		if(spawner_left.v < to_sp(SHIELD_CENTER_X)) {
			spawner_velocity_x.v += to_sp(0.0625f);
		} else {
			spawner_velocity_x.v -= to_sp(0.0625f);
		}
		if(spawner_top.v < to_sp(SHIELD_CENTER_Y)) {
			spawner_velocity_y.v += to_sp(0.0625f);
		} else {
			spawner_velocity_y.v -= to_sp(0.0625f);
		}
		grc_sloppy_unput(spawner_left.to_pixel(), spawner_top.to_pixel());
		for(int i = (eggs_alive - 1); i >= 0; i--) {
			bird_put_8(egg.left[i], egg.top[i], C_EGG);
		}
		spawner_left.v += spawner_velocity_x.v;
		spawner_top.v += spawner_velocity_y.v;
		if((boss_phase_frame % pattern_state.interval) == 0) {
			if(overlap_xy_lrtb_le_ge(
				spawner_left.v, spawner_top.v,
				to_sp(0.0), to_sp(0.0f),
				to_sp(RES_X - SPAWNCROSS_W), to_sp(RES_Y - SPAWNCROSS_H)
			)) {
				egg.left[eggs_alive] = spawner_left.to_pixel();
				egg.top[eggs_alive] = spawner_top.to_pixel();
				bird_put_8(
					spawner_left.to_pixel(), spawner_top.to_pixel(), C_EGG
				);
				eggs_alive++;
			}
		}
		grc_put_8(
			spawner_left.to_pixel(),
			spawner_top.to_pixel(),
			GRC_SLOT_SPAWNCROSS,
			(boss_phase_frame % SPAWNCROSS_CELS),
			((boss_phase_frame % 4) + 2)
		);
	} else if(boss_phase_frame < 400) {
		if(boss_phase_frame == 200) {
			if(abs(spawner_velocity_y.v) < to_sp(0.25f)) {
				spawner_velocity_y.v = (spawner_velocity_y.v < 0)
					? to_sp(-0.25f)
					: to_sp( 0.25f);
			}
			if(abs(spawner_velocity_x.v) < to_sp(0.25f)) {
				spawner_velocity_x.v = (spawner_velocity_x.v < 0)
					? to_sp(-0.25f)
					: to_sp( 0.25f);
			}
			for(int i = (eggs_alive - 1); i >= 0; i--) {
				vector2(velocity.x, velocity.y, 3, ((irand() & 0x7F) + 0x80));
				Birds.spawn(egg.left[i], egg.top[i], velocity.x, velocity.y);
			}
			wand_lower_both();
			pellet_group = static_cast<bird_pellet_group_t>(
				irand() % (BPG_6_RING + 1) // excluding random rain here
			);
		}

		if(overlap_xy_lrtb_le_ge(
			spawner_left, spawner_top,
			to_sp(-SPAWNCROSS_W), to_sp(-SPAWNCROSS_H),
			to_sp(RES_X), to_sp(RES_Y)
		)) {
			grc_sloppy_unput(spawner_left.to_pixel(), spawner_top.to_pixel());
			spawner_left.v += spawner_velocity_x.v;
			spawner_top.v += spawner_velocity_y.v;

			// ZUN bug: ZUN suddenly forgot that [spawner] uses subpixels, not
			// pixels. Comparing its position to the pixel equivalent of
			// 	((RES_X - SPAWNCROSS_W), (RES_Y - SPAWNCROSS_H))
			// effectively clips the top-left coordinate of the spawner to a
			// screen-space rectangle from (0, 0) to (38, 23). Since that's
			// quite far from its actual position, the code below never gets
			// executed.
			if(overlap_xy_lrtb_le_ge(
				spawner_left, spawner_top,
				0, 0,
				(RES_X - SPAWNCROSS_W), // should be subpixels
				(RES_Y - SPAWNCROSS_H) // should be subpixels
			)) {
				// ZUN bug: Also, here. Quite ironic given that there's a
				// correct version of this blitting call above… Copy-pasta
				// confirmed.
				grc_put_8(
					spawner_left, // should be pixels
					spawner_top, // should be pixels
					GRC_SLOT_SPAWNCROSS,
					(boss_phase_frame % SPAWNCROSS_CELS),
					((boss_phase_frame % 4) + 2)
				);
			}
		}
		if(((boss_phase_frame % 16) == 0) || ((boss_phase_frame % 16) == 2)) {
			Birds.fire(pellet_group);
		}
	} else if(boss_phase_frame == 400) {
		boss_phase_frame = 0;
		wand_raise_animation_done = false;
	}
}
