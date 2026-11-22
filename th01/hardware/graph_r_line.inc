void graph_r_line(
	screen_x_t left,
	vram_y_t top,
	screen_x_t right,
	vram_y_t bottom,
	vc2 col
)
{
	register vram_offset_t vram_offset;
	int i;
	screen_x_t x_cur;
	vram_y_t y_cur;
	pixel_t w, h;
	int error;
	int y_direction;
	int order_tmp;
	vram_x_t x_vram;
	vram_y_t y_vram;
	dots16_t pixels;

#define lerp(m, x) static_cast<int>(m * static_cast<float>(x))

#define clip_lerp_min(low, high, lerp_point, slope_dividend, minimum) \
	if(low < minimum) { \
		if(high < minimum) { \
			return; \
		} \
		lerp_point += lerp((slope_dividend / (high - low)), (minimum - low)); \
		low = minimum; \
	}
#define clip_lerp_max(low, high, lerp_point, slope_dividend, maximum) \
	if(high > maximum) { \
		if(low > maximum) { \
			return; \
		} \
		lerp_point -= lerp((slope_dividend / (high - low)), (high - maximum)); \
		high = maximum; \
	}

#ifdef TH01_NATIVE_ORB_PHYSICS
#define unput32_at(vram_offset) native_line_restore32(vram_offset)
#else
#define unput32_at(vram_offset) { \
	Planar<dots32_t> page1; \
	page_access(1);	VRAM_SNAP_PLANAR(page1, vram_offset, 32); \
	page_access(0);	VRAM_PUT_PLANAR(vram_offset, page1, 32); \
}

#endif

#define plot_loop(\
	step_var, step_len, step_increment, \
	plotted_var, plotted_len, plotted_increment \
) \
	error = (step_len >> 1); \
	for(i = 0; i <= step_len; i++) { \
		/* Advanced past the VRAM cursor? */ \
		if((x_cur >> 3) != x_vram || (y_vram != y_cur)) { \
			vram_offset = (y_vram * ROW_SIZE) + x_vram; \
			if(!graph_r_unput) { \
				grcg_put(vram_offset, pixels, 16); \
				pixels = 0; \
			} else { \
				/* ZUN bug: Getting here with a [vram_offset] of 0x0000 will \
				 * cause a 4-byte write starting at 0xFFFF. On the 80286 and \
				 * later CPUs, offset overflows within an instruction are \
				 * illegal even in Real Mode, and will raise a General \
				 * Protection Fault. \
				 * As of May 2022, Anex86 is the only PC-98 emulator to \
				 * correctly replicate this behavior of real hardware, \
				 * though. */ \
				vram_offset--; \
				unput32_at(vram_offset); \
			} \
			y_vram = y_cur; \
			x_vram = (x_cur >> BYTE_BITS); \
		} \
		pixels |= (graph_r_pattern >> (x_cur & BYTE_MASK)); \
		pixels |= (graph_r_pattern << (16 - (x_cur & BYTE_MASK))); \
		error -= plotted_len; \
		step_var += step_increment; \
		if(error < 0) { \
			error += step_len; \
			plotted_var += plotted_increment; \
		} \
	} \
	if(graph_r_unput) { \
		goto restore_last; \
	} \
	goto end;

	if(!graph_r_unput && (left == right)) {
		graph_r_vline(left, top, bottom, col);
		return;
	}
	if(!graph_r_unput && (top == bottom)) {
		graph_r_hline(left, right, top, col);
		return;
	}
	if(left > right) {
		order_tmp = left;
		left = right;
		right = order_tmp;
		order_tmp = top;
		top = bottom;
		bottom = order_tmp;
	}

	clip_lerp_min(left, right, top, (bottom - top), 0);
	clip_lerp_max(left, right, bottom, (bottom - top), (RES_X - 1));
	clip_lerp_min(top, bottom, left, (right - left), 0);
	clip_lerp_max(top, bottom, right, (right - left), (RES_Y - 1));

	// This division is safe at this point.
	#define slope_y ((right - left) / (bottom - top))

	if(bottom < 0) {
		right += lerp(slope_y, 0 - bottom);
		bottom = 0;
	}
	if(top > (RES_Y - 1)) {
		left -= lerp(slope_y, top - (RES_Y - 1));
		top = (RES_Y - 1);
	}
	graph_r_last_line_end.x = right;
	graph_r_last_line_end.y = bottom;
	x_cur = left;
	y_cur = top;
	y_direction = (top < bottom) ? 1 : -1;
	w = right - left;
	h = (bottom - top) * y_direction;
	pixels = 0;
	x_vram = (x_cur >> BYTE_BITS);
	y_vram = y_cur;

	if(!graph_r_unput) {
		grcg_setcolor_rmw(col);
	}
	if(w > h) {
		plot_loop(x_cur, w, 1, y_cur, h, y_direction);
	} else {
		plot_loop(y_cur, h, y_direction, x_cur, w, 1);
	}
restore_last:
	// ZUN bug: Off-by-one error, as [x_cur] and [y_cur] are one past the
	// intended right / bottom coordinates after the plot_loop. Should have
	// calculated [vram_offset] from [x_vram] and [y_vram] just like the
	// plot_loop, since those values are directly updated for the next VRAM
	// byte after a blit, and would thus be correct here as well.
	//
	// This way, the offset could potentially end up at [right = 640] or
	// [bottom = -1]. Both together are not only the same as (0, 0) and thus
	// wrap from the right edge of VRAM back to the left one, but also trigger
	// the same General Protection Fault seen in the plot_loop itself.
	vram_offset = vram_offset_shift(x_cur, y_cur) - 1;
	unput32_at(vram_offset);
end:
	if(!graph_r_unput) {
		grcg_off_func();
	}

#undef slope_y
#undef plot_loop
#undef unput32_at
#undef clip_lerp_max
#undef clip_lerp_min
#undef lerp
}

