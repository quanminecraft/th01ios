void near dottedcircle_unput_update_render(
	screen_x_t center_x,
	screen_y_t center_y,
	int frame_1based,
	int interval,
	pixel_t radius_step,
	vc2 col,
	pixel_t radius_initial,
	int frames
)
{
	static pixel_t radius_prev;
	static bool16 active;

	#define radius_cur \
		(((frame_1based / interval) * radius_step) + radius_initial)

	if(frame_1based == 1) {
		active = true;
		// Yup, no rendering call this frame.
	} else {
		if((active != true) || ((frame_1based % interval) != 0)) {
			return;
		}
		shape_circle_sloppy_unput(center_x, center_y, radius_prev, 0x01);
		if(frame_1based >= frames) {
			active = false;
			return;
		}
		shape_circle_put(center_x, center_y, radius_cur, col, 0x01);
	}
	radius_prev = radius_cur;

	#undef radius_cur
}
