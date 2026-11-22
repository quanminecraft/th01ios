void shape_ellipse_arc_sloppy_unput(
	screen_x_t center_x,
	vram_y_t center_y,
	pixel_t radius_x,
	pixel_t radius_y,
	unsigned char angle_step,
	unsigned char angle_start,
	unsigned char angle_end
)
{
	int angle; // 16 bits to correctly work for an [angle_end] of 0xFF

	 // ZUN bug: Should all be initialized
	screen_x_t cur_x;
	vram_y_t cur_y;
	#ifdef TH01_NATIVE_ORB_PHYSICS
    screen_x_t prev_x=-32768;
#else
    screen_x_t prev_x;
#endif
	#ifdef TH01_NATIVE_ORB_PHYSICS
    vram_y_t prev_y=-32768;
#else
    vram_y_t prev_y;
#endif

	if(angle_start > angle_end) {
		return;
	}

	for(angle = angle_start; angle <= angle_end; angle += angle_step) {
		cur_x = polar_x(center_x, radius_x, angle);
		cur_y = polar_y(center_y, radius_y, angle);
		if(
			(prev_y != cur_y) ||
			((prev_x >> BYTE_BITS) != (cur_x >> BYTE_BITS))
		) {
			egc_copy_rect_1_to_0_16(cur_x, cur_y, BYTE_DOTS, 1);
			prev_x = cur_x;
			prev_y = cur_y;
		}
	}
}

