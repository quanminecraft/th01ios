static const pixel_t SURROUND_AREA_W = ((PLAYFIELD_W * 3) / 10);
static const pixel_t SURROUND_AREA_H = ((PLAYFIELD_H * 8) / 21);

#define surround_random_left(relative_to) ( \
	(relative_to.cur_left + (irand() % SURROUND_AREA_W)) - \
	((SURROUND_AREA_W - relative_to.w_static()) / 2) \
)

#define surround_random_top(relative_to) ( \
	(relative_to.cur_top + (irand() % SURROUND_AREA_H)) + \
	relative_to.surround_area_offset_y(SURROUND_AREA_H) \
)
// -------------

// Random teleport and bat movement coordinates
// --------------------------------------------

inline screen_x_t elis_playfield_random_left(void) {
	return (PLAYFIELD_LEFT + (irand() % (PLAYFIELD_W - GIRL_W)));
}

inline screen_y_t elis_playfield_random_top(void) {
	return playfield_rand_y(0.0f, (17 / 42.0f));
}
// --------------------------------------------

