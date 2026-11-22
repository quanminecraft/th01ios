inline int lives_col(int i) {
	return (i % LIVES_PER_ROW);
}

inline screen_y_t lives_top(int i) {
	return (LIVES_TOP + ((i / LIVES_PER_ROW) * ROW_H));
}

#define lives_bg(func, left, i) \
	ptn_id_and_quarter_from_i( \
		func, left, lives_col(i), lives_top(i), PTN_BG_LIVES, i \
	)

#define lives_put(left, i) \
	sprite_put(left, lives_col(i), lives_top(i), PTN_HUD, PTN_LIFE_QUARTER)

#define bombs_bg(func, left, i) \
	ptn_id_and_quarter_from_i(func, left, i, BOMBS_TOP, PTN_BG_BOMBS, i)

#define bombs_put(left, i) \
	sprite_put(left, i, BOMBS_TOP, PTN_HUD, PTN_BOMB_QUARTER)

#define bg_snap_and_put(left, top, func_bg, func_sprite, var) \
	for(int i = 0; i < var; i++) { \
		func_bg(bg_snap, left, i); \
		func_sprite(left, i); \
	} \
	/* Yet we don't accomodate cheaters here, and only copy a single row? */ \
	graph_copy_hud_row_0_to_1_8(left, top, var * COL_W);

#define put_change(left, top, func_bg, func_sprite, var_prev, var_new) \
	if(var_prev > var_new) { \
		for(int i = var_new; i < var_prev; i++) { \
			page_access(1);	func_bg(bg_put, left, i); \
			page_access(0);	func_bg(bg_put, left, i); \
		} \
	} else if(var_prev < var_new) { \
		for(int i = var_prev; i < var_new; i++) { \
			func_bg(bg_snap, left, i); \
			page_access(0); \
			func_sprite(left, i); \
		} \
		graph_copy_hud_row_0_to_1_8(left, top, var_new * COL_W); \
	} \

void lives_bg_snap_and_put(void)
{
	bg_snap_and_put(LIVES_LEFT, LIVES_TOP, lives_bg, lives_put, rem_lives);
}

void hud_lives_put(int prev)
{
	put_change(LIVES_LEFT, LIVES_TOP, lives_bg, lives_put, prev, rem_lives);
}

void bombs_bg_snap_and_put(void)
{
	bg_snap_and_put(BOMBS_LEFT, BOMBS_TOP, bombs_bg, bombs_put, rem_bombs);
}

void hud_bombs_put(int prev)
{
	put_change(BOMBS_LEFT, BOMBS_TOP, bombs_bg, bombs_put, prev, rem_bombs);
}

#undef put_change
#undef bg_snap_and_put
/// ---------------

