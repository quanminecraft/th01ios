void CBossEntity::pos_set(
	screen_x_t left,
	screen_y_t top,
	screen_x_t move_clamp_left,
	screen_x_t move_clamp_right,
	screen_y_t move_clamp_top,
	screen_y_t move_clamp_bottom
)
{
	this->cur_left = left;
	this->cur_top = top;
	this->move_clamp.left = move_clamp_left;
	this->move_clamp.right = move_clamp_right;
	this->move_clamp.top = move_clamp_top;
	this->move_clamp.bottom = move_clamp_bottom;
}

void CBossEntity::locked_move_unput_and_put_8(
	pixel_t delta_x, pixel_t delta_y, int lock_frames
)
{
	if(lock_frame == 0) {
		move(delta_x, delta_y);

		screen_x_t unput_left = (delta_x > 0)
			? ((prev_left / BYTE_DOTS) * BYTE_DOTS)
			: (((cur_left / BYTE_DOTS) * BYTE_DOTS) + (vram_w * BYTE_DOTS));
		egc_copy_rect_1_to_0_16(unput_left, prev_top, 8, h);

		vram_y_t unput_top = (cur_top > prev_top)
			? prev_top
			: (cur_top + h);
		egc_copy_rect_1_to_0_16(
			prev_left, unput_top, (vram_w << 3), abs(cur_top - prev_top)
		);

		unput_and_put_8(cur_left, cur_top, bos_image);

		lock_frame = 1;
	} else if(lock_frame >= lock_frames) {
		lock_frame = 0;
	} else {
		lock_frame++;
	}
}

void CBossEntity::locked_move_and_put_8(
	pixel_t delta_x, pixel_t delta_y, int lock_frames
)
{
	if(lock_frame == 0) {
		move(delta_x, delta_y);
		put_8(cur_left, cur_top, bos_image);
		lock_frame = 1;
	} else if(lock_frame >= lock_frames) {
		lock_frame = 0;
	} else {
		lock_frame++;
	}
}

bool16 CBossEntity::hittest_orb(void) const
{
	if(hitbox_orb_inactive == true) {
		return false;
	}
	if(
		((hitbox_orb.left + cur_left - ORB_HITBOX_RIGHT) <= orb_prev_left) &&
		((hitbox_orb.right + cur_left - ORB_HITBOX_LEFT) >= orb_prev_left) &&
		((cur_top + hitbox_orb.top - ORB_HITBOX_BOTTOM) <= orb_prev_top) &&
		((cur_top + hitbox_orb.bottom - ORB_HITBOX_TOP) >= orb_prev_top)
	) {
		return true;
	}
	return false;
}

