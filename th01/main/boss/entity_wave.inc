#define wave_func(len, amp, phase, call) { \
	int t = phase; \
	for(pixel_t bos_y = 0; bos_y < h; bos_y++) { \
		screen_x_t left = (wave_x(amp, t) + cur_left); \
		t += (0x100 / len); \
		call \
	} \
}

void CBossEntity::wave_put(int image, int len, pixel_t amp, int phase) const
{
	wave_func(len, amp, phase, {
		put_1line(left, (cur_top + bos_y), image, bos_y);
	});
}

void CBossEntity::wave_sloppy_unput(int len, pixel_t amp, int phase) const
{
	wave_func(len, amp, phase, {
		egc_copy_rect_1_to_0_16(left, (cur_top + bos_y), w_aligned(), 1);
	});
}
