void shape_ellipse_arc_put(
	screen_x_t center_x,
	vram_y_t center_y,
	pixel_t radius_x,
	pixel_t radius_y,
	vc2 col,
	unsigned char angle_step,
	unsigned char angle_start,
	unsigned char angle_end
)
{
	int angle; // 16 bits to correctly work for an [angle_end] of 0xFF
	dots8_t cache_dots = 0;
	vram_offset_t cache_vram_offset = -1;

	// ZUN landmine: Leaves the GRCG activated if the return condition below is
	// `true`. Should be done after that branch instead.
	grcg_setcolor_rmw(col);

	// Note that due to the cache, this function doesn't end up drawing
	// anything if [angle_start] == [angle_end] either. (This actually happens
	// in Elis' pattern_pellets_along_circle().)
	if(angle_start > angle_end) {
		return;
	}

	for(angle = angle_start; angle <= angle_end; angle += angle_step) {
		vram_offset_t vram_offset;
		screen_x_t cur_x = polar_x(center_x, radius_x, angle);
		vram_y_t   cur_y = polar_y(center_y, radius_y, angle);

		vram_offset = vram_offset_shift(cur_x, cur_y);
		if(cache_vram_offset != vram_offset) {
			if(
				(cur_x >= 0) && (cur_x < RES_X) &&
				(cache_vram_offset >= 0) && (cache_vram_offset < PLANE_SIZE)
			) {
				grcg_put(cache_vram_offset, cache_dots, 8);
			}
			cache_dots = (0x80 >> (cur_x & BYTE_MASK));
			cache_vram_offset = vram_offset;
		} else {
			cache_dots |= (0x80 >> (cur_x & BYTE_MASK));
		}
	}
	grcg_off_func();
}

