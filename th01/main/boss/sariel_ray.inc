void near spawnray_unput_and_put(
	screen_x_t origin_x, vram_y_t origin_y,
	screen_x_t target_x, vram_y_t target_y,
	vc2 col
)
{
	static screen_x_t target_prev_x = -PIXEL_NONE;
	static vram_y_t target_prev_y = -PIXEL_NONE;
	spawnray_unput_and_put_func(
		target_prev_x, target_prev_y,
		origin_x, origin_y, target_x, target_y, col, true
	);
}

inline void spawnray_reset(void) {
	spawnray_unput_and_put(0, 0, 0, 0, SPAWNRAY_RESET);
}
