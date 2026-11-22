inline main_ptn_id_t orb_anim_cel(void) {
	return static_cast<main_ptn_id_t>(
		PTN_ORB + (orb_rotation_frame / ORB_FRAMES_PER_CEL)
	);
}

void orb_and_pellets_and_stage_unput_update_render__vsync_wait(int stage_id)
{
	if(!orb_in_portal) {
		orb_move_x(orb_velocity_x);
		orb_cur_top += orb_velocity_y_update();
		orb_frames_outside_portal++;

		if(orb_is_moving_left()) {
			orb_rotation_frame++;
		}
		if(orb_is_moving_right()) {
			orb_rotation_frame--;
		}

		// ZUN quirk: The fourth cel is only visible for a single frame?
		if(orb_rotation_frame >= ((ORB_CELS * ORB_FRAMES_PER_CEL) - 2)) {
			orb_rotation_frame = 0;
		}
		if(orb_rotation_frame < 0) {
			orb_rotation_frame = ((ORB_CELS - 1) * ORB_FRAMES_PER_CEL);
		}

		if(orb_cur_top > ORB_TOP_MAX) {
			orb_force_new(COEFFICIENT_OF_RESTITUTION, OF_BOUNCE_FROM_SURFACE);
			orb_cur_top = ORB_TOP_MAX;
			cardcombo_cur = 0;
		}
		if(orb_cur_top < ORB_TOP_MIN) {
			orb_force_new(0, OF_BOUNCE_FROM_TOP);
			orb_cur_top = ORB_TOP_MIN;
		}
	}

	frame_delay(1);
	Pellets.unput_update_render();

	// Stage object collision is explicitly deactivated on boss stages? Since
	// there's still a card combo metric on the TOTLE screen though, these
	// lines add another piece of evidence that that screen was shown for
	// regular stages as well, at one point in development.
	if(!stage_is_boss(stage_id)) {
		cards_hittest(stage_id);
		obstacles_update_and_render(false);
	}

	if(!orb_in_portal) {
		orb_player_hittest(OR_NONE);
	}

	if(!orb_in_portal) {
		// ZUN landmine: Shouldn't you unblit *before* updating the rotation
		// frame? While it's OK (and even sort of essential) to assume that all
		// Orb sprites are shaped identically (as they are in the original
		// data), ordering the calls like this still violates common sense.
		ptn_unput_8(orb_prev_left, orb_prev_top, orb_anim_cel());
	}

	if(!stage_is_boss(stage_id)) {
		cards_update_and_render();
	}

	if(!orb_in_portal && !player_is_hit) {
		ptn_put_8(orb_cur_left, orb_cur_top, orb_anim_cel());
	}

	orb_prev_left = orb_cur_left;
	orb_prev_top = orb_cur_top;
}
