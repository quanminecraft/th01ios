void near wand_lowered_snap(void)
{
	int ptn_x;
	int ptn_y;
	screen_x_t left = anm_wand.left;
	screen_y_t top = anm_wand.top;
	int image = 0;

	ptn_snap_rect_from_1_8(
		left, top, WAND_W, WAND_H, PTN_SLOT_WAND_LOWERED, image, ptn_x, ptn_y
	);
}

void near wand_lowered_put(void)
{
	int ptn_x;
	int ptn_y;
	screen_x_t left;
	screen_y_t top;
	int image;

	image = 0;
	left = anm_wand.left;
	top = anm_wand.top;

	ptn_put_rect_noalpha_8(
		left, top, WAND_W, WAND_H, PTN_SLOT_WAND_LOWERED, image, ptn_x, ptn_y
	)
}
